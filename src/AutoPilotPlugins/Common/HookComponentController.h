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
    Q_INVOKABLE void hookReset();

private:
    Vehicle* _vehicle;
    QElapsedTimer _since_start_timer;
    void _setActiveVehicle(Vehicle* vehicle);
    void _sendHookStepCommand(int num_steps);
    void _sendHookResetCommand();
    void _sendNamedValueFloat(const char* name, float value);
};

#endif
