#include "HookComponentController.h"
#include "MultiVehicleManager.h"
#include <iostream>

QGC_LOGGING_CATEGORY(HookComponent, "HookComponent")

HookComponentController::HookComponentController() : _vehicle(nullptr) 
{
    _since_start_timer.start(); // start the boot timer to timestamp the mavlink messages

    MultiVehicleManager *manager = qgcApp()->toolbox()->multiVehicleManager();
    connect(manager, &MultiVehicleManager::activeVehicleChanged, this, &HookComponentController::_setActiveVehicle);
    this->_setActiveVehicle(manager->activeVehicle());
}

void
HookComponentController::hookStep(int num_steps)
{
    _sendHookStepCommand(num_steps);
}

void
HookComponentController::hookReset(int position)
{
    _sendHookResetCommand(position);
}

void
HookComponentController::_setActiveVehicle(Vehicle* vehicle)
{
    _vehicle = vehicle;
}

void
HookComponentController::_sendHookStepCommand(int num_steps)
{
    _sendNamedValueFloat("hook_stp", num_steps);
}
void
HookComponentController::_sendHookResetCommand(int position)
{
    _sendNamedValueFloat("hook_rst", position);
}

void
HookComponentController::_sendNamedValueFloat(const char* name, float value)
{
    if (_vehicle) {
        std::cout << "sending named value float with name " << name << " and value " << value << std::endl;
        WeakLinkInterfacePtr weakLink = _vehicle->vehicleLinkManager()->primaryLink();
        if (!weakLink.expired()) {
            SharedLinkInterfacePtr sharedLink = weakLink.lock();

            qCDebug(HookComponent) << "Sending NAMED_VALUE_FLOAT with name " << name << " and value " << value;
            mavlink_message_t msg;
            mavlink_msg_named_value_float_pack_chan(
                        qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
                        qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
                        sharedLink->mavlinkChannel(),
                        &msg,
                        this->_since_start_timer.elapsed(),
                        name,
                        value);
            _vehicle->sendMessageOnLinkThreadSafe(sharedLink.get(), msg);
        }
    }
    else
    {
        std::cout << "not sending named value float since no vehicle connected" << std::endl;
    }
}
