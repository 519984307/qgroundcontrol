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
    _sendHookCommand(1);
}
void
LandingStationControlButtonsController::hookClose(void)
{
    _sendHookCommand(0);
}
void
LandingStationControlButtonsController::hookSecured(void)
{
    _sendHookCommand(2);
}

void
LandingStationControlButtonsController::beltStop(void)
{
    _sendBeltCommand(0);
}
void
LandingStationControlButtonsController::beltUp(void)
{
    _sendBeltCommand(2);
}
void
LandingStationControlButtonsController::beltDown(void)
{
    _sendBeltCommand(-2);
}
void
LandingStationControlButtonsController::beltRotateLeft(void)
{
    _sendBeltCommand(-1);
}
void
LandingStationControlButtonsController::beltRotateRight(void)
{
    _sendBeltCommand(1);
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
LandingStationControlButtonsController::_sendTimeoutCommand(float value)
{
    _sendNamedValueFloat("lan_tmt", value);
}
void
LandingStationControlButtonsController::_sendSpeedCommand(int value)
{
    _sendNamedValueFloat("lan_spd", static_cast<float>(value)/100);
}
void
LandingStationControlButtonsController::_sendDeliveryCommand(int value)
{
    _sendNamedValueFloat("delivery", value);
}
void
LandingStationControlButtonsController::_sendHookCommand(int value)
{
    _sendNamedValueFloat("hook_msg", value);
}
void
LandingStationControlButtonsController::_sendBeltCommand(int value)
{
    _sendNamedValueFloat("lan_act", value);
}
void
LandingStationControlButtonsController::_sendBeltLevelCommand()
{
    _sendNamedValueFloat("lan_lvl", 0.0f);
}

void
LandingStationControlButtonsController::_sendNamedValueFloat(const char* name, float value)
{
    if (_vehicle) {
        std::cout << "sending named value float with name " << name << " and value " << value << std::endl;
        WeakLinkInterfacePtr weakLink = _vehicle->vehicleLinkManager()->primaryLink();
        if (!weakLink.expired()) {
            SharedLinkInterfacePtr sharedLink = weakLink.lock();

            qCDebug(LandingStationControl) << "Sending NAMED_VALUE_FLOAT with name " << name << " and value " << value;
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
