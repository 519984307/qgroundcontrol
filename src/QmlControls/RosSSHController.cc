#include "RosSSHController.h"

RosSSHController::RosSSHController(void)
{
    _current_glider_name = "";
    _current_username = "";
    _current_ip = "";
    _current_timeout_ms = 25000; // usually our SSH shells take a loooooong time to just log in
    _newest_result = -1;
    _newest_stdout = "";
    _status = -1;
    _ssh_thread = nullptr;
}

void
RosSSHController::startROS(QString gliderName) {
    newConnection(gliderName);
    if (startCommand("") != 0) {
        setStatus(2);
    };
}

void
RosSSHController::newConnection(QString gliderName) {
    std::string ip = "";
    SettingsManager* _settingsManager = qgcApp()->toolbox()->settingsManager();

    std::string username = _settingsManager->flyViewSettings()->JetsonUsername()->rawValue().toString().toStdString();

    // Get the IP from the settings
    std::string glider_name = gliderName.toStdString();
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

    setConnection(glider_name, username, ip);
}

void
RosSSHController::setTimeout(int milliseconds)
{
    _current_timeout_ms = milliseconds;
}

int
RosSSHController::startCommand(QString command)
{
    if (_current_glider_name.empty()) {
        qCDebug(RosSSHLogger)<<"Connection is not set, ignoring command " << _current_glider_name.c_str();
        return 1;
    } else if (_ssh_thread) {
        qCDebug(RosSSHLogger)<<"SSH command already in progress, ignoring new command";
        return 1;
    }

    std::string command_str = command.toStdString();
    qCDebug(RosSSHLogger)<<"Starting command: " << command_str.c_str() << "\n";

    // Run the command in a separate thread
    _ssh_thread = std::make_shared<RosSSHThread>();

    // This will set the username and hostname in the thread
    _ssh_thread->setConnection(_current_username, _current_ip);
    _ssh_thread->setTimeout(_current_timeout_ms);
    _ssh_thread->setCmd(command_str);

    connect(_ssh_thread.get(), &RosSSHThread::resultReady, this, &RosSSHController::handleSSHResults);
    // connect(workerThread, &RosSSHThread::finished, workerThread, &QObject::deleteLater);

    setStatus(1);
    _ssh_thread->start();
    return 0;
}

std::string
RosSSHController::readStdOut()
{
    if (_ssh_thread)
    {
        return _ssh_thread->stdOut();
    }
    else
    {
        return _newest_stdout;
    }
}

void
RosSSHController::setStatus(int status)
{
    _status = status;
    emit statusChanged(_status);
    if (status != 1) {
        qCDebug(RosSSHLogger)<<"Stdout was " << readStdOut().c_str();
    }
}

void
RosSSHController::handleSSHResults(int result)
{
    _newest_result = result;
    _newest_stdout = _ssh_thread->stdOut();

    if(result==2)
    {
        std::string setup_command = "ssh-copy-id "+_current_username+"@"+_current_ip;
        std::string osType = QSysInfo::productType().toStdString();
        if(osType == "windows" || osType == "winrt")
        {
            setup_command = "type %userprofile%\\.ssh\\id_rsa.pub | ssh "+_current_username+"@"+_current_ip+" \"cat >> .ssh/authorized_keys\"";
        }
        std::string message = std::string("Make sure that your network setup is correct, and that you have run the SSH setup commands for this glider.\n\n") +
                              "If you didn't run the SSH setup, do the following steps CAREFULLY:\n" +
                              "1- If you have never used SSH on this machine, run \"ssh-keygen -b 4096\" (should be executed only once, choose \"Cancel\" if the command asks you to overwrite an existing SSH key)\n" +
                              "2- Run \""+setup_command+"\" to copy your SSH identity to the glider, you will be prompted to enter the glider's password.\n\n" +
                              "Stdout of command was:\n" + _newest_stdout;

        setStatus(2);

        qgcApp()->showAppMessage(
            tr(message.c_str()),
            tr("SSH to glider failed")
            );
    }
    else
    {
        setStatus(0);
    }

    _ssh_thread = nullptr;
    emit newResult(_newest_result);
}

void
RosSSHController::setConnection(std::string glider_name, std::string username, std::string ip)
{
    if (!checkIp(ip)) {
        std::string error_message = "You have set the IP of \""+ glider_name + "\" to \""+ip+"\", which is an invalid IP address. Please set a valid IP in Application Settings";
        qgcApp()->showAppMessage(
            tr(error_message.c_str()),
            tr("Invalid IP address")
            );
        resetConnection();
    } else {
        qCDebug(RosSSHLogger)<<"Setting connection to " << glider_name.c_str() << " " << username.c_str() << " " << ip.c_str();
        _current_glider_name = glider_name;
        _current_ip = ip;
        _current_username = username;
    }
}

void
RosSSHController::resetConnection()
{
    _current_glider_name = "";
    _current_ip = "";
    _current_username = "";
    setStatus(-1);
}

bool
RosSSHController::checkIp(std::string ip) {
    return std::regex_match(ip, _ip_regex);
}
