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
        
        if(!currentVehicle) return "No Vehicle";

        // Case when we are connected
        if(currentVehicle.landingStationConnected.value) return qsTr("LS connected", "Connected to Landing Station");

        // Get the current distance info when we are disconnected
        var dist_info = "";
        const d = new Date();
        let time = d.getTime()/1000;
        console.log(currentVehicle.landingStationDistanceLastTime.value);
        console.log(currentVehicle.landingStationDistance.value);
        console.log(time);

        if(time - currentVehicle.landingStationDistanceLastTime.value < 3)
        {
            dist_info = " (" + Math.round(currentVehicle.landingStationDistance.value * 100) / 100 + "m)";
        }

        return qsTr("LS disconnected" + dist_info, "Landing Station not connected");
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


