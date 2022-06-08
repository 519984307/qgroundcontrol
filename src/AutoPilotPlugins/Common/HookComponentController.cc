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
    SharedLinkInterfacePtr sharedLink = _getLink();
    if(!sharedLink) {
        return;
    }

    int sign;
    if(num_steps >= 0) {
        sign = 1;
    } else {
        sign = -1;
    }

    uint16_t steps_to_send;
    steps_to_send |= sign << 15; // first bit is sign
    // other bits are number of steps. We just mask out the first bit of
    // num_steps, if someone wants to move more than 32768 steps at once they
    // have other problems
    steps_to_send |= (num_steps && 0x7fff);

    mavlink_message_t msg;
    mavlink_msg_hook_command_pack_chan(
        qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
        qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
        sharedLink->mavlinkChannel(),
        &msg,
        254,
        0,
        this->_since_start_timer.elapsed()*1000,
        HOOK_COMMAND::HOOK_CORRECTION,
        steps_to_send);
    _sendMavlinkMessage(msg, sharedLink);
}
void
HookComponentController::_sendHookResetCommand(int position)
{
    SharedLinkInterfacePtr sharedLink = _getLink();
    if(!sharedLink) {
        return;
    }

    _vehicle->hookPosition()->setRawValue(position);

    // to reset we send a hook command message with the right position and
    // something not 0 for the steps
    mavlink_message_t msg;
    mavlink_msg_hook_command_pack_chan(
        qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
        qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
        sharedLink->mavlinkChannel(),
        &msg,
        254,
        0,
        this->_since_start_timer.elapsed()*1000,
        position,
        1);
    _sendMavlinkMessage(msg, sharedLink);
}

SharedLinkInterfacePtr
HookComponentController::_getLink()
{
    if (_vehicle) {
        WeakLinkInterfacePtr weakLink = _vehicle->vehicleLinkManager()->primaryLink();
        if (!weakLink.expired()) {
            return weakLink.lock();
        }
    }
    qCDebug(HookComponent) << "Tried to send mavlink message but failed";
    return nullptr;
}

//----------------------------------------------------------------------------------------
void
HookComponentController::_sendMavlinkMessage(mavlink_message_t& msg, SharedLinkInterfacePtr link_interface)
{
    _vehicle->sendMessageOnLinkThreadSafe(link_interface.get(), msg);
}