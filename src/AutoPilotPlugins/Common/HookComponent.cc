/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "HookComponent.h"

HookComponent::HookComponent(Vehicle* vehicle, AutoPilotPlugin* autopilot, QObject* parent) :
    VehicleComponent(vehicle, autopilot, parent),
    _name(tr("Hook"))
{

}

QString HookComponent::name(void) const
{
    return _name;
}

QString HookComponent::description(void) const
{
    return tr("Hook Setup is used to manually test the hook control and adjust the hook position.");
}

QString HookComponent::iconResource(void) const
{
    return QStringLiteral("/qmlimages/HookComponentIcon.svg");
}

bool HookComponent::requiresSetup(void) const
{
    return false;
}

bool HookComponent::setupComplete(void) const
{
    return true;
}

QStringList HookComponent::setupCompleteChangedTriggerList(void) const
{
    return QStringList();
}

QUrl HookComponent::setupSource(void) const
{
    return QUrl::fromUserInput(QStringLiteral("qrc:/qml/HookComponent.qml"));
}

QUrl HookComponent::summaryQmlSource(void) const
{
    return QUrl();
}
