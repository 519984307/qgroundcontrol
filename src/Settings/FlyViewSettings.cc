/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "FlyViewSettings.h"

#include <QQmlEngine>
#include <QtQml>

DECLARE_SETTINGGROUP(FlyView, "FlyView")
{
    qmlRegisterUncreatableType<FlyViewSettings>("QGroundControl.SettingsManager", 1, 0, "FlyViewSettings", "Reference only"); \
}

// TODO: Add a fact here to adapt the instrument panel to the hotfix (we need to add it in JSON, CC, and HEADER files)

DECLARE_SETTINGSFACT(FlyViewSettings, guidedMinimumAltitude)
DECLARE_SETTINGSFACT(FlyViewSettings, guidedMaximumAltitude)
DECLARE_SETTINGSFACT(FlyViewSettings, showLogReplayStatusBar)
DECLARE_SETTINGSFACT(FlyViewSettings, alternateInstrumentPanel)
DECLARE_SETTINGSFACT(FlyViewSettings, showAdditionalIndicatorsCompass)
DECLARE_SETTINGSFACT(FlyViewSettings, lockNoseUpCompass)

DECLARE_SETTINGSFACT(FlyViewSettings, fwdTelemetryFix)
DECLARE_SETTINGSFACT(FlyViewSettings, TelemetryFixAMat)
DECLARE_SETTINGSFACT(FlyViewSettings, TelemetryFixBMat)


DECLARE_SETTINGSFACT(FlyViewSettings, maxGoToLocationDistance)
DECLARE_SETTINGSFACT(FlyViewSettings, keepMapCenteredOnVehicle)
DECLARE_SETTINGSFACT(FlyViewSettings, showSimpleCameraControl)
DECLARE_SETTINGSFACT(FlyViewSettings, showObstacleDistanceOverlay)

// Start ROS settings
DECLARE_SETTINGSFACT(FlyViewSettings, JetsonUsername)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider1Name)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider1IP)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider2Name)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider2IP)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider3Name)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider3IP)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider4Name)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider4IP)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider5Name)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider5IP)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider6Name)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider6IP)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider7Name)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider7IP)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider8Name)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider8IP)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider9Name)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider9IP)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider10Name)
DECLARE_SETTINGSFACT(FlyViewSettings, Glider10IP)

DECLARE_SETTINGSFACT(FlyViewSettings, unicastVideoSink)
DECLARE_SETTINGSFACT(FlyViewSettings, lastROSIP)

DECLARE_SETTINGSFACT(FlyViewSettings, landingStationControlsVisible)
DECLARE_SETTINGSFACT(FlyViewSettings, landingStationTimeout)
DECLARE_SETTINGSFACT(FlyViewSettings, landingStationSpeed)





