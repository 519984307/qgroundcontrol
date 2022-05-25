#define LIBSSH_STATIC 1
// #include <libssh/libsshpp.hpp>
#define SSH_NO_CPP_EXCEPTIONS
#include <string>
#include <libssh/libsshpp.hpp>
#include "RosSSHThread.h"

QGC_LOGGING_CATEGORY(RosSSHLogger, "RosSSHLogger")


void
RosSSHThread::run() {
    int result = 2;
    // First, we create an SSH session
    ssh::Session* my_ssh_session = new ssh::Session();
    my_ssh_session->setOption(SSH_OPTIONS_HOST, _connection.c_str());
    

    // TODO: Deal with this case
    // if (my_ssh_session == NULL)
    //     emit resultReady(2);
    //     return;

    // ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, _connection.c_str());

    int rc = my_ssh_session->connect();
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Error connecting to localhost: %s\n",
                ssh_get_error(my_ssh_session));
        emit resultReady(2);
        return;
    }
    rc = my_ssh_session->userauthPublickeyAuto();
    // rc = ssh_userauth_publickey_auto(my_ssh_session, NULL, NULL);
    if (rc == SSH_AUTH_ERROR)
    {
        fprintf(stderr, "Authentication failed: %s\n",
        ssh_get_error(my_ssh_session));
        emit resultReady(2);
        return;
    }
    // // Then we create a channel associated with the session
    // ssh_channel channel;
    char buffer[256];
    int nbytes;
    ssh::Channel* channel = new ssh::Channel(*my_ssh_session);
    if (channel == NULL) {
        emit resultReady(2);
        return;
    }
    rc = channel->openSession();
    if (rc != SSH_OK)
    {
        delete channel;
        emit resultReady(2);
        return;
    }
    // We can use the channel to execute a remote command here

    if(_cmd != "") {
        rc =channel->requestExec(_cmd.c_str());
        if (rc != SSH_OK)
        {
            channel->close();
            delete channel;
            emit resultReady(2);
            return;
        }

        // TODO: Add a timeout here as a fourth argument
        nbytes = channel->read(buffer, sizeof(buffer), true);
        while (nbytes > 0)
        {
            if (write(1, buffer, nbytes) != (unsigned int) nbytes)
            {
                channel->close();
                delete channel;
                emit resultReady(2);
                return;
            }
            nbytes = channel->read(buffer, sizeof(buffer), true);
        }
        if (nbytes < 0)
        {
            channel->close();
            delete channel;
            emit resultReady(2);
            return;
        }
        
    }
    // // Close the channel
    channel->sendEof();
    channel->close();
    delete channel;
    my_ssh_session->disconnect();
    delete my_ssh_session;

    
    // QString program = tr("ssh");
    // QStringList arguments;
    // /*
    //     arguments:
    //         "-tt" this enforces the creation of a terminal, in case we want to read the STDOUT
    //         "-o PasswordAuthentication=no" makes the ssh fail if a password is needed
    //         (password needed means that the setup is not correct)
    // */
    // arguments << "-tt" << "-o" << "PasswordAuthentication=no" << _connection.c_str() << _cmd.c_str();

    // qCDebug(RosSSHLogger)<<"Executing command " << program << arguments << "\n";
    // QProcess *myProcess = new QProcess();

    // myProcess->setProcessChannelMode(QProcess::MergedChannels);
    // myProcess->start(program, arguments);
    // myProcess->closeWriteChannel();

    // myProcess->waitForFinished(_timeout_ms);
    // int result = myProcess->state() == QProcess::Running ? 0 : (myProcess->exitCode() == 0 ? 0:2);

    // save stdout for later in case someone wants to read it
    // _stdout_str = myProcess->readAll().toStdString();
    
    // myProcess->kill();
    qCDebug(RosSSHLogger)<<"SSH done from SSH thread with code " << result << "\n";
    emit resultReady(0);
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
