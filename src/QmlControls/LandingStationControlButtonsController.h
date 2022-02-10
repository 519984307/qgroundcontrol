#ifndef LandingStationControlButtonsController_H
#define LandingStationControlButtonsController_H

#include "QGCApplication.h"
#include <QQuickItem>
#include <QElapsedTimer>

Q_DECLARE_LOGGING_CATEGORY(LandingStationControl)

class LandingStationControlButtonsController : public QObject
{
    Q_OBJECT

public:
    LandingStationControlButtonsController();
    Q_INVOKABLE double getTimeout();
    Q_INVOKABLE void setTimeout(float timeout);
    Q_INVOKABLE int getSpeed();
    Q_INVOKABLE void setSpeed(int speed);
    Q_INVOKABLE void beltStop();
    Q_INVOKABLE void beltUp();
    Q_INVOKABLE void beltDown();
    Q_INVOKABLE void beltRotateLeft();
    Q_INVOKABLE void beltRotateRight();

    Q_PROPERTY(int timeout READ getTimeout WRITE setTimeout NOTIFY timeoutChanged);
    Q_PROPERTY(int speed READ getSpeed WRITE setSpeed NOTIFY speedChanged);

signals:
    void timeoutChanged(float timeout);
    void speedChanged(int speed);

private:
    float _timeout_s;
    int _speed_perc;
    Vehicle* _vehicle;
    QElapsedTimer _since_start_timer;
    void _setActiveVehicle(Vehicle* vehicle);
    void _sendTimeoutCommand(float value);
    void _sendSpeedCommand(int value);
    void _sendBeltCommand(int value);
    void _sendNamedValueFloat(const char* name, float value);
};

#endif