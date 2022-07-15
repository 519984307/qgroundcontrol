/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "SettingsGroup.h"

class FlyViewSettings : public SettingsGroup
{
    Q_OBJECT
public:
    FlyViewSettings(QObject* parent = nullptr);

    DEFINE_SETTING_NAME_GROUP()

    DEFINE_SETTINGFACT(guidedMinimumAltitude)
    DEFINE_SETTINGFACT(guidedMaximumAltitude)
    DEFINE_SETTINGFACT(showLogReplayStatusBar)
    DEFINE_SETTINGFACT(alternateInstrumentPanel)
    DEFINE_SETTINGFACT(showAdditionalIndicatorsCompass)


    DEFINE_SETTINGFACT(fwdTelemetryFix)
    DEFINE_SETTINGFACT(TelemetryFixAMat)
    DEFINE_SETTINGFACT(TelemetryFixBMat)


    DEFINE_SETTINGFACT(lockNoseUpCompass)
    DEFINE_SETTINGFACT(maxGoToLocationDistance)
    DEFINE_SETTINGFACT(keepMapCenteredOnVehicle)
    DEFINE_SETTINGFACT(showSimpleCameraControl)
    DEFINE_SETTINGFACT(showObstacleDistanceOverlay)

    // Start ROS settings
    DEFINE_SETTINGFACT(Glider1Name)
    DEFINE_SETTINGFACT(Glider1IP)
    DEFINE_SETTINGFACT(Glider2Name)
    DEFINE_SETTINGFACT(Glider2IP)
    DEFINE_SETTINGFACT(Glider3Name)
    DEFINE_SETTINGFACT(Glider3IP)
    DEFINE_SETTINGFACT(Glider4Name)
    DEFINE_SETTINGFACT(Glider4IP)
    DEFINE_SETTINGFACT(Glider5Name)
    DEFINE_SETTINGFACT(Glider5IP)
    DEFINE_SETTINGFACT(Glider6Name)
    DEFINE_SETTINGFACT(Glider6IP)
    DEFINE_SETTINGFACT(Glider7Name)
    DEFINE_SETTINGFACT(Glider7IP)
    DEFINE_SETTINGFACT(Glider8Name)
    DEFINE_SETTINGFACT(Glider8IP)
    DEFINE_SETTINGFACT(Glider9Name)
    DEFINE_SETTINGFACT(Glider9IP)
    DEFINE_SETTINGFACT(Glider10Name)
    DEFINE_SETTINGFACT(Glider10IP)

    DEFINE_SETTINGFACT(JetsonUsername)

    DEFINE_SETTINGFACT(unicastVideoSink)

    DEFINE_SETTINGFACT(lastROSIP)

    // Lanidng station control settings
    DEFINE_SETTINGFACT(landingStationControlsVisible)
    DEFINE_SETTINGFACT(landingStationTimeout)
    DEFINE_SETTINGFACT(landingStationSpeed)
};
