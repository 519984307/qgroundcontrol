#ifndef RosSSHThread_H
#define RosSSHThread_H

#include "QGCApplication.h"
#include <QString>
#include <QThread>
#include <QProcess>

Q_DECLARE_LOGGING_CATEGORY(RosSSHLogger)

class RosSSHThread : public QThread
{
    Q_OBJECT
    void run() override;
public:
    void setConnection(std::string username, std::string ip);
    void setCmd(std::string cmd);
    void setTimeout(int milliseconds);
    std::string stdOut();
    virtual ~RosSSHThread(){};

signals:
    void resultReady(const int &s);

private:
    std::string _connection = "";
    std::string _cmd = "";
    std::string _stdout_str = "";
    int _timeout_ms = 0;
};

#endif // RosSSHThread_H
