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
    Q_INVOKABLE void hookClose();
    Q_INVOKABLE void hookOpen();
    Q_INVOKABLE void hookHooked();
    Q_INVOKABLE void deliverPackage();
    Q_INVOKABLE void beltStop();
    Q_INVOKABLE void beltUp();
    Q_INVOKABLE void beltDown();
    Q_INVOKABLE void beltRotateLeft();
    Q_INVOKABLE void beltRotateRight();
    Q_INVOKABLE void beltLevel();

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
    SharedLinkInterfacePtr _getLink();
    void _setActiveVehicle(Vehicle* vehicle);
    void _sendTimeoutCommand(float value);
    void _sendSpeedCommand(int value);
    void _sendHookCommand(int value);
    void _sendDeliveryCommand(int value);
    void _sendBeltCommand(int value);
    void _sendBeltLevelCommand();
    void _sendNamedValueFloat(const char* name, float value);
    void _vehicleSetHookChanged(uint8_t position);
    void _sendMavlinkMessage(mavlink_message_t& msg, SharedLinkInterfacePtr link_interface);
};

#endif
