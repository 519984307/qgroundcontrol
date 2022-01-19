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

// Label control that shows the connection status to the Landing Station
QGCLabel {
    id:     _root
    text:   updateText()

    function updateText()
    {
        /*console.log(currentVehicle.landingStationConnected.value)
        console.log(currentVehicle.landingStationConnected.rawValue)*/
        if(!currentVehicle) return "No Vehicle";
        return currentVehicle.landingStationConnected.value ?  qsTr("LS connected", "Connected to Landing Station") : qsTr("LS disconnected", "Landing Station not connected");
    }
    property var    currentVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    

    /*Item {
       Timer {
               interval: 100; running: true; repeat: true
               onTriggered: _root.text = updateText();
             }
    }*/

    property real mouseAreaLeftMargin:    0 
}


