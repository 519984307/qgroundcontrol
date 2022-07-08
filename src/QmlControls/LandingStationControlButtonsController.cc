#include "LandingStationControlButtonsController.h"
#include <iostream>
#include "MultiVehicleManager.h"

QGC_LOGGING_CATEGORY(LandingStationControl, "LandingStationControl")

LandingStationControlButtonsController::LandingStationControlButtonsController() : _vehicle(nullptr) 
{
    _since_start_timer.start(); // start the boot timer to timestamp the mavlink messages

    MultiVehicleManager *manager = qgcApp()->toolbox()->multiVehicleManager();
    connect(manager, &MultiVehicleManager::activeVehicleChanged, this, &LandingStationControlButtonsController::_setActiveVehicle);
    this->_setActiveVehicle(manager->activeVehicle());
}

double
LandingStationControlButtonsController::getTimeout()
{
    return _timeout_s;
}
void
LandingStationControlButtonsController::setTimeout(float timeout)
{
    _timeout_s = timeout;
    emit timeoutChanged(_timeout_s);
    _sendTimeoutCommand(_timeout_s);
}

int
LandingStationControlButtonsController::getSpeed()
{
    return _speed_perc;
}
void
LandingStationControlButtonsController::setSpeed(int speed)
{
    _speed_perc = speed;
    emit speedChanged(_speed_perc);
    _sendSpeedCommand(_speed_perc);
}

void
LandingStationControlButtonsController::hookOpen(void)
{
    _sendHookCommand(HOOK_COMMAND::HOOK_OPEN);
    _vehicleSetHookChanged(1);
}
void
LandingStationControlButtonsController::hookClose(void)
{
    _sendHookCommand(HOOK_COMMAND::HOOK_CLOSE);
    _vehicleSetHookChanged(0);
}
void
LandingStationControlButtonsController::hookHooked(void)
{
    _sendHookCommand(HOOK_COMMAND::HOOK_SECURE);
    _vehicleSetHookChanged(0);
}

void
LandingStationControlButtonsController::deliverPackage()
{
    _sendDeliveryCommand();
}

void
LandingStationControlButtonsController::beltStop(void)
{
    _sendBeltCommand(LS_BELT_COMMAND::LS_BELT_CMD_STOP);
}
void
LandingStationControlButtonsController::beltUp(void)
{
    _sendBeltCommand(LS_BELT_COMMAND::LS_BELT_CMD_BOTH_UP);
}
void
LandingStationControlButtonsController::beltDown(void)
{
    _sendBeltCommand(LS_BELT_COMMAND::LS_BELT_CMD_BOTH_DOWN);
}
void
LandingStationControlButtonsController::beltRotateLeft(void)
{
    _sendBeltCommand(LS_BELT_COMMAND::LS_BELT_CMD_RIGHT_UP);
}
void
LandingStationControlButtonsController::beltRotateRight(void)
{
    _sendBeltCommand(LS_BELT_COMMAND::LS_BELT_CMD_LEFT_UP);
}
void
LandingStationControlButtonsController::beltLevel(void)
{
    _sendBeltLevelCommand();
}

void
LandingStationControlButtonsController::_setActiveVehicle(Vehicle* vehicle)
{
    _vehicle = vehicle;
    _sendSpeedCommand(_speed_perc);
    _sendTimeoutCommand(_timeout_s);
}

void
LandingStationControlButtonsController::_sendTimeoutCommand(float time_out)
{
    SharedLinkInterfacePtr sharedLink = _getLink();
    if(!sharedLink) {
        return;
    }

    mavlink_message_t msg;
    mavlink_msg_ls_time_out_pack_chan(
        qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
        qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
        sharedLink->mavlinkChannel(),
        &msg,
        253,
        253,
        this->_since_start_timer.elapsed()*1000,
        time_out);
    _sendMavlinkMessage(msg, sharedLink);
}

void
LandingStationControlButtonsController::_sendSpeedCommand(int speed)
{
    SharedLinkInterfacePtr sharedLink = _getLink();
    if(!sharedLink) {
        return;
    }

    mavlink_message_t msg;
    mavlink_msg_ls_speed_pack_chan(
        qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
        qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
        sharedLink->mavlinkChannel(),
        &msg,
        253,
        253,
        this->_since_start_timer.elapsed()*1000,
        speed);
    _sendMavlinkMessage(msg, sharedLink);
}

void
LandingStationControlButtonsController::_sendDeliveryCommand()
{
    SharedLinkInterfacePtr sharedLink = _getLink();
    if(!sharedLink) {
        return;
    }

    mavlink_message_t msg;
    mavlink_msg_parcel_delivery_pack_chan(
        qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
        qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
        sharedLink->mavlinkChannel(),
        &msg,
        254,
        254,
        this->_since_start_timer.elapsed()*1000);
    _sendMavlinkMessage(msg, sharedLink);
}

void
LandingStationControlButtonsController::_sendHookCommand(int command)
{
    SharedLinkInterfacePtr sharedLink = _getLink();
    if(!sharedLink) {
        return;
    }

    mavlink_message_t msg;
    mavlink_msg_hook_command_pack_chan(
        qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
        qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
        sharedLink->mavlinkChannel(),
        &msg,
        254,
        254,
        this->_since_start_timer.elapsed()*1000,
        command,
        0);
    _sendMavlinkMessage(msg, sharedLink);
}

void
LandingStationControlButtonsController::_sendBeltCommand(int command)
{
    SharedLinkInterfacePtr sharedLink = _getLink();
    if(!sharedLink) {
        return;
    }

    mavlink_message_t msg;
    mavlink_msg_ls_belt_command_pack_chan(
        qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
        qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
        sharedLink->mavlinkChannel(),
        &msg,
        253,
        253,
        this->_since_start_timer.elapsed()*1000,
        100,
        command,
        0.0f);
    _sendMavlinkMessage(msg, sharedLink);
}

void
LandingStationControlButtonsController::_sendBeltLevelCommand()
{
    SharedLinkInterfacePtr sharedLink = _getLink();
    if(!sharedLink) {
        return;
    }

    mavlink_message_t msg;
    mavlink_msg_ls_level_pack_chan(
        qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
        qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
        sharedLink->mavlinkChannel(),
        &msg,
        254,
        254,
        this->_since_start_timer.elapsed()*1000);
    _sendMavlinkMessage(msg, sharedLink);
}

SharedLinkInterfacePtr
LandingStationControlButtonsController::_getLink()
{
    if (_vehicle) {
        WeakLinkInterfacePtr weakLink = _vehicle->vehicleLinkManager()->primaryLink();
        if (!weakLink.expired()) {
            return weakLink.lock();
        }
    }
    qCDebug(LandingStationControl) << "Tried to send mavlink message but failed";
    return nullptr;
}

//----------------------------------------------------------------------------------------
void
LandingStationControlButtonsController::_sendMavlinkMessage(mavlink_message_t& msg, SharedLinkInterfacePtr link_interface)
{
    _vehicle->sendMessageOnLinkThreadSafe(link_interface.get(), msg);
}

void
LandingStationControlButtonsController::_vehicleSetHookChanged(uint8_t position) {
    _vehicle->hookPosition()->setRawValue(position);
    // set status to unknown since we don't know if we can reach the position
    // when the hook does or does not reach the target, the status will be set
    // accordingly from ROS
    _vehicle->hookStatus()->setRawValue(2);
}

