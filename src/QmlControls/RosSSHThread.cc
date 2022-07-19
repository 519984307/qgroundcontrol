#define LIBSSH_STATIC 1
#define SSH_NO_CPP_EXCEPTIONS
#include <string>
#include <libssh/libsshpp.hpp>
#include "RosSSHThread.h"

QGC_LOGGING_CATEGORY(RosSSHLogger, "RosSSHLogger")


void
RosSSHThread::run() {
    // First, we create an SSH session
    ssh::Session* my_ssh_session = new ssh::Session();
    my_ssh_session->setOption(SSH_OPTIONS_HOST, _connection.c_str());
    
    qCDebug(RosSSHLogger)<<"Created SSH session\n";

    if (my_ssh_session == NULL) {
        qCDebug(RosSSHLogger)<<"Unexpected ERROR: SSH session NULL\n";
        emit resultReady(2);
        return;
    }

    int rc = my_ssh_session->connect();
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Error connecting to host: %s\n",
                ssh_get_error(my_ssh_session));
        emit resultReady(2);
        return;
    }
    qCDebug(RosSSHLogger)<<"Connected the SSH session to the host\n";
    rc = my_ssh_session->userauthPublickeyAuto();

    if (rc == SSH_AUTH_ERROR)
    {
        fprintf(stderr, "Authentication failed: %s\n",
        ssh_get_error(my_ssh_session));
        emit resultReady(2);
        return;
    }
    // Then we create a channel associated with the session
    qCDebug(RosSSHLogger)<<"SSH authentication successful\n";

    char buffer[256];
    int nbytes;
    ssh::Channel* channel = new ssh::Channel(*my_ssh_session);
    if (channel == NULL) {
        emit resultReady(2);
        return;
    }
    qCDebug(RosSSHLogger)<<"Created SSH channel\n";
    rc = channel->openSession();
    if (rc != SSH_OK)
    {
        delete channel;
        emit resultReady(2);
        return;
    }
    qCDebug(RosSSHLogger)<<"Opened Channel session\n";

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
        qCDebug(RosSSHLogger)<<"Started command execution\n";
        // Read command output with a timeout
        int timeout_ms = 25000;

        nbytes = channel->read(buffer, sizeof(buffer), true, timeout_ms);
        while (nbytes > 0)
        {
            if (write(1, buffer, nbytes) != (unsigned int) nbytes)
            {
                channel->close();
                delete channel;
                emit resultReady(2);
                return;
            }
            nbytes = channel->read(buffer, sizeof(buffer), true, timeout_ms);
        }
        if (nbytes < 0)
        {
            channel->close();
            delete channel;
            emit resultReady(2);
            return;
        }
        
    }
    // else we just wait for some time then close the SSH session
    else {
        sleep(15);
    }
    // Close the channel
    channel->sendEof();
    channel->close();
    delete channel;
    my_ssh_session->disconnect();
    delete my_ssh_session;

    qCDebug(RosSSHLogger)<<"SSH done from SSH thread with code 0";
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
