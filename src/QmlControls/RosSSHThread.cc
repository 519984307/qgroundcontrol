#include "RosSSHThread.h"

QGC_LOGGING_CATEGORY(RosSSHLogger, "RosSSHLogger")

void
RosSSHThread::run() {
    QString program = tr("ssh");
    QStringList arguments;
    /*
        arguments:
            "-tt" this enforces the creation of a terminal, in case we want to read the STDOUT
            "-o PasswordAuthentication=no" makes the ssh fail if a password is needed
            (password needed means that the setup is not correct)
    */
    arguments << "-tt" << "-o" << "PasswordAuthentication=no" << _connection.c_str() << _cmd.c_str();

    qCDebug(RosSSHLogger)<<"Executing command " << program << arguments << "\n";
    QProcess *myProcess = new QProcess();

    myProcess->setProcessChannelMode(QProcess::MergedChannels);
    myProcess->start(program, arguments);
    myProcess->closeWriteChannel();

    myProcess->waitForFinished(_timeout_ms);
    int result = myProcess->state() == QProcess::Running ? 0 : (myProcess->exitCode() == 0 ? 0:2);

    // save stdout for later in case someone wants to read it
    _stdout_str = myProcess->readAll().toStdString();
    
    myProcess->kill();
    qCDebug(RosSSHLogger)<<"SSH done from SSH thread with code " << result << "\n";
    emit resultReady(result);
}

void
RosSSHThread::setConnection(std::string username, std::string ip)
{
    _connection = username + "@" + ip;
    qCDebug(RosSSHLogger)<<"Set connection to " << _connection.c_str() << "\n";
}

void
RosSSHThread::setCmd(std::string cmd)
{
    _cmd = cmd;
    qCDebug(RosSSHLogger)<<"Set command to " << _cmd.c_str() << "\n";
}

void
RosSSHThread::setTimeout(int milliseconds)
{
    _timeout_ms = milliseconds;
    qCDebug(RosSSHLogger)<<"Set timeout to " << _timeout_ms << "\n";
}

std::string
RosSSHThread::stdOut()
{
    return _stdout_str;
}
