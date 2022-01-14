#include"StartROSMenuController.h"
#include<sstream>
#include<regex>

StartROSMenuController::StartROSMenuController(void)
{
    _status = -1;
}

bool
StartROSMenuController::connectSSH(QString text)
{
    std::string ip = "";
    SettingsManager* _settingsManager = qgcApp()->toolbox()->settingsManager();

    std::string username = _settingsManager->flyViewSettings()->JetsonUsername()->rawValue().toString().toStdString();

    // Get the IP from the settings
    std::string glider_name = text.toStdString();
    if(glider_name == "Mark11")
    {
        ip = _settingsManager->flyViewSettings()->Mark11IP()->rawValue().toString().toStdString();
    }
    else if(glider_name == "Mark12")
    {
        ip = _settingsManager->flyViewSettings()->Mark12IP()->rawValue().toString().toStdString();
    }
    else if(glider_name == "Mark13")
    {
        ip = _settingsManager->flyViewSettings()->Mark13IP()->rawValue().toString().toStdString();
    }
    else if(glider_name == "Skeleton12")
    {
        ip = _settingsManager->flyViewSettings()->Skeleton12IP()->rawValue().toString().toStdString();
    }
    else if(glider_name == "Skeleton13")
    {
        ip = _settingsManager->flyViewSettings()->Skeleton13IP()->rawValue().toString().toStdString();
    }

    // Assemble the ssh username and hostname
    std::ostringstream stream;
    stream << username <<"@" << ip; //<< " -p " << password;

    std::string cmd = stream.str();

    // Check the IP validity before running SSH
    std::string ip_regex = "(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])";
    if(!std::regex_match(ip, std::regex(ip_regex)))
    {
        std::string error_message = "You have set the IP of \""+ glider_name + "\" to \""+ip+"\", which is an invalid IP address. Please set a valid IP in Application Settings";
        qgcApp()->showAppMessage(
            tr(error_message.c_str()),
            tr("Invalid IP address")
            );
        return false;
    }
    

    set_status(1);
    _current_glider_name = glider_name;
    _current_ip = ip;
    _current_username = username;
    
    // Run the command in a separate thread
    RosSSHThread *workerThread = new RosSSHThread();
    // This will set the username and hostname in the thread
    workerThread->set_cmd(cmd);

    connect(workerThread, &RosSSHThread::resultReady, this, &StartROSMenuController::handleSSHResults);
    connect(workerThread, &RosSSHThread::finished, workerThread, &QObject::deleteLater);

    workerThread->start();
    return true;
}

void StartROSMenuController::handleSSHResults(int val)
{
    std::cout << "Got signal from the ssh thread\n";
    set_status(val);
}


int StartROSMenuController::status()
{
    return _status;
}

int StartROSMenuController::reset_status()
{
    set_status(-1);
    return 0;
}

void StartROSMenuController::set_status(int val)
{
    _status = val;
    std::cout << "Status changed to " << val << "\n";
    emit statusChanged(val);
    if(status()==2)
    {
        std::string setup_command = "ssh-copy-id "+_current_username+"@"+_current_ip;
        std::string osType = proQSysInfo::productType().toStdString();
        if(osType == "windows" || osType == "winrt")
        {
            setup_command = "type $env:USERPROFILE\.ssh\id_rsa.pub | ssh "+_current_username+"@"+_current_ip+" \"cat >> .ssh/authorized_keys\"";        
        }
        std::string message = "Make sure that your network setup is correct, and that you have run the SSH setup commands for this glider.\n\nIf you didn't run the SSH setup, do the following steps CAREFULLY:\n1- If you have never used SSH on this machine, run \"ssh-keygen -b 4096\" (should be executed only once, choose \"Cancel\" if the command asks you to overwrite an existing SSH key)\n2- Run \""+setup_command+"\" to copy your SSH identity to the glider, you will be prompted to enter the glider's password.";
        qgcApp()->showAppMessage(
            tr(message.c_str()),
            tr("SSH to glider failed")
            );
    }
    else if(status() == 0)
    {
        qgcApp()->toolbox()->settingsManager()->flyViewSettings()->lastROSIP()->setRawValue(QVariant(QString::fromStdString(_current_ip)));
    }
}