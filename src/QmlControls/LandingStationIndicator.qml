/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick                      2.12
import QtQuick.Controls             2.12

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0

// Label control whichs pop up a flight mode change menu when clicked
QGCLabel {
    id:     _root
    text:   currentVehicle.landingStationConnected ? qsTr("LS disconnected", "Landing Station not connected") : qsTr("LS connected", "Connected to Landing Station")

    property var    currentVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    property real   mouseAreaLeftMargin:    0
}
