#ifndef StartROSMenuController_H
#define StartROSMenuController_H

#include<iostream>
#include <QTimer>
#include "QGCApplication.h"
#include <QQuickItem>
#include <QCursor>
#include "SettingsManager.h"
#include <QSysInfo> 

class StartROSMenuController : public QObject
{
    Q_OBJECT

public:
    StartROSMenuController();
    Q_INVOKABLE bool connectSSH(QString text);
    /*
        status gives one of the following:
        -1: Undefined
        0: Successful
        1: Running
        2: Failed
    */
    Q_INVOKABLE int status();
    Q_INVOKABLE int reset_status();
    Q_PROPERTY(int status READ status WRITE set_status NOTIFY statusChanged)

signals:
    void statusChanged(int status);

private:
    void set_status(int val);
    int _status;
    void handleSSHResults(int val);
    std::string _current_glider_name;
    std::string _current_ip;
    std::string _current_username;

};

#endif

#ifndef RosSSHThread_H
#define RosSSHThread_H

#include <QThread>
#include <QProcess>
#include <regex>

class RosSSHThread : public QThread
{
    Q_OBJECT
    void run() override {
        std::string osType = proQSysInfo::productType().toStdString();
        std::cout<<osType<<"\n";
        if(osType == "windows" || osType == "winrt")
        {
            std::cout<<"Windows confirmed\n";
        }
        int result;
        QString program = tr("ssh");
        QStringList arguments;
        /*
            arguments:
             "-tt" this enforces the creation of a terminal, in case we want to read the STDOUT
             "-o PasswordAuthentication=no" makes the ssh fail if a password is needed
                (password needed means that the setup is not correct)
        */
        arguments << "-tt" << "-o" << "PasswordAuthentication=no" << _cmd.c_str();

        QProcess *myProcess = new QProcess();

        myProcess->setProcessChannelMode(QProcess::MergedChannels);
        myProcess->start(program, arguments);

        /*
            Here we watch the STDOUT,
            deprecated use: Checking if we were prompted for a password

            Keeping the code since it might be used in the future.
        */
        myProcess->closeWriteChannel();
        myProcess->waitForReadyRead(3000);
        std::string stdout_str = myProcess->readAll().toStdString();
        std::string password_prompt_regex = _cmd + "'s password:";
        std::cout<<"Got the following STDOUT : " << stdout_str;
        if(std::regex_match(stdout_str, std::regex(password_prompt_regex)))
        {
            result = 2;
        }


        else
        {
            myProcess->waitForFinished(22000);
            result = myProcess->state() == QProcess::Running? 0 : (myProcess->exitCode() == 0? 0:2);
        }
        
        myProcess->kill();
        std::cout<<"SSH done from SSH thread with code " << result << "\n";
        emit resultReady(result);
    }
public:
    void set_cmd(std::string cmd)
    {
        _cmd = cmd;
    }
    virtual ~RosSSHThread(){};
signals:
    void resultReady(const int &s);
private:
    std::string _cmd = "";
};

#endif