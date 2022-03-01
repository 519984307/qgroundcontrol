#ifndef RosSSHController_H
#define RosSSHController_H

#include <RosSSHThread.h>

#include "QGCApplication.h"
#include <QString>
#include <QSysInfo>
#include <QThread>
#include <QProcess>
#include "SettingsManager.h"
#include <regex>

class RosSSHController : public QObject
{
    Q_OBJECT

public:
    RosSSHController();
    Q_INVOKABLE void startROS(QString gliderName);
    Q_INVOKABLE void newConnection(QString gliderName);
    Q_INVOKABLE void resetConnection();
    Q_INVOKABLE void setTimeout(int milliseconds);
    Q_INVOKABLE int startCommand(QString command);
    Q_INVOKABLE std::string readStdOut();
    Q_INVOKABLE int status() {return _status;}
    Q_INVOKABLE void setStatus(int status);

    Q_PROPERTY(int status READ status WRITE setStatus NOTIFY statusChanged);

signals:
    void statusChanged(const int &s);
    void newResult(const int &r);

private:
    std::string _current_glider_name;
    std::string _current_ip;
    std::string _current_username;
    int _current_timeout_ms;
    int _newest_result;
    int _status; // -1: disconnected, 0: idle, 1: running, 2: failed
    std::string _newest_stdout;
    std::shared_ptr<RosSSHThread> _ssh_thread;

    const std::regex _ip_regex = std::regex("(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])");

    void handleSSHResults(int result);
    void setConnection(std::string glider_name, std::string username, std::string ip);
    bool checkIp(std::string ip);
};

#endif // RosSSHController_H
