#ifndef HookComponentController_H
#define HookComponentController_H

#include "QGCApplication.h"
#include <QElapsedTimer>

Q_DECLARE_LOGGING_CATEGORY(HookComponent)

class HookComponentController : public QObject
{
    Q_OBJECT

public:
    HookComponentController();
    Q_INVOKABLE void hookStep(int num_steps);
    Q_INVOKABLE void hookReset(int positioin);

private:
    Vehicle* _vehicle;
    QElapsedTimer _since_start_timer;
    void _setActiveVehicle(Vehicle* vehicle);
    void _sendHookStepCommand(int num_steps);
    void _sendHookResetCommand(int position);
    void _sendNamedValueFloat(const char* name, float value);
    SharedLinkInterfacePtr _getLink();
    void _sendMavlinkMessage(mavlink_message_t& msg, SharedLinkInterfacePtr link_interface);
};

#endif
