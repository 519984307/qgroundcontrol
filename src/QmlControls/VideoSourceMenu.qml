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

import QGroundControl                       1.0
import QGroundControl.FactSystem            1.0
import QGroundControl.FactControls          1.0
import QGroundControl.Controls              1.0
import QGroundControl.ScreenTools           1.0
import QGroundControl.MultiVehicleManager   1.0
import QGroundControl.Palette               1.0
import QGroundControl.Controllers           1.0
import QGroundControl.SettingsManager       1.0

// Label control whichs pop up a flight mode change menu when clicked
QGCLabel {
    id:     _root
    text:   _root.get_stream_name(QGroundControl.settingsManager.videoSettings.udpPort.rawValue)

    property var    currentVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    property real   mouseAreaLeftMargin:    0

    Menu {
        id: flightModesMenu
    }

    Component {
        id: flightModeMenuItemComponent

        MenuItem {
            enabled: true
            onTriggered: QGroundControl.settingsManager.videoSettings.udpPort.value = _root.get_port(text)
        }
    }

    property var flightModesMenuItems: []
    property var ports: [5601, 5602]
    property var stream_names: ["Regular image (5601)", "Debug image (5602)"]

    function get_stream_name(port) {
        var i = 0;
        for (i = 0; i<ports.length; i++)
        {
            if(parseInt(port)==ports[i]) return stream_names[i];
        }
        return qsTr(`Custom image (${port})`)
    }

    function get_port(stream_name) {
        var i = 0;
        for (i = 0; i<ports.length;i++){
            if(stream_name==stream_names[i]) return ports[i];
        }
        return 5601; // Default port (regular image)
    }

    function updateFlightModesMenu() {
        console.log(QGroundControl.settingsManager.videoSettings.udpPort.rawValue)
        // console.log(VideoSettings.udp264VideoSource)
        if (currentVehicle) {
            var i;
            // Remove old menu items
            for (i = 0; i < flightModesMenuItems.length; i++) {
                flightModesMenu.removeItem(flightModesMenuItems[i])
            }
            flightModesMenuItems.length = 0
            // Add new items
            
            for (i = 0; i < ports.length; i++) {
                var menuItem = flightModeMenuItemComponent.createObject(null, { "text": stream_names[i]})
                flightModesMenuItems.push(menuItem)
                flightModesMenu.insertItem(i, menuItem)
            }
        }
    }

    Component.onCompleted: _root.updateFlightModesMenu()

    Connections {
        target:                 QGroundControl.multiVehicleManager
        onActiveVehicleChanged: _root.updateFlightModesMenu()
    }

    MouseArea {
        id:                 mouseArea
        visible:            currentVehicle && currentVehicle.flightModeSetAvailable
        anchors.leftMargin: mouseAreaLeftMargin
        anchors.fill:       parent
        onClicked:          flightModesMenu.popup((_root.width - flightModesMenu.width) / 2, _root.height)
    }
}
