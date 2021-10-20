/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

// Might be useful to have this later.

import QtQuick                              2.11
import QtQuick.Controls                     2.4

import QGroundControl                       1.0
import QGroundControl.FactSystem            1.0
import QGroundControl.FactControls          1.0
import QGroundControl.Controls              1.0
import QGroundControl.ScreenTools           1.0
import QGroundControl.MultiVehicleManager   1.0
import QGroundControl.Palette               1.0
import QGroundControl.Controllers           1.0
import QGroundControl.SettingsManager       1.0

//-------------------------------------------------------------------------
//-- Mode Indicator
QGCComboBox {
    anchors.verticalCenter: parent.verticalCenter
    alternateText:          qsTr("TEST")
    model:                  _udpRangeStr
    font.pointSize:         ScreenTools.mediumFontPointSize
    currentIndex:           -1
    sizeToContents:         true

    property bool showIndicator: true

    // property var _activeVehicle:    QGroundControl.multiVehicleManager.activeVehicle
    // property var _flightModes:      _activeVehicle ? _activeVehicle.flightModes : [ ]
    property var _udpRange: [5600, 5601, 5602, 5603, 5604]
    property var _udpRangeStr: [qsTr("5600"), qsTr("5601"), qsTr("5602"), qsTr("5603"), qsTr("5604")]

    onActivated: {
        _videoSettings.udpPort = _udpRange[index]
        currentIndex = -1
    }

    property var test: _videoSettings.udpPort
}
