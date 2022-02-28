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
    text:   mainText()


    property var    currentVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    property var    controller:             null
    property real   mouseAreaLeftMargin:    0
    property var    sshStatus:              -1
    property var    waitingForResult:       false

    Connections {
        target: controller
        function onStatusChanged(value) {
            _root.setSSHStatus(value);
        }
    }

    Menu {
        id: flightModesMenu
    }

    Component {
        id: flightModeMenuItemComponent

        MenuItem {
            enabled: true
            onTriggered: {controller.newConnection(text); waitingForResult = true; flightModesMenu.dismiss()}
        }
    }

    Item {
       Timer {
               interval: 7000; running: true; repeat: true
               onTriggered: _root.reset_ssh_status();
             }
    } 

    function reset_ssh_status()
    {
        if(sshStatus!=-1 && sshStatus!=1) {
            setSSHStatus(-1);
        }
    }

    function setSSHStatus(value) {
        sshStatus = value;
        text = mainText()
        // make sure we only react to results that we triggered ourselves
        if (waitingForResult && value == 2) {
            // SSH failed, reset connection (since we couldn't connect)
            controller.resetConnection()
        }
        waitingForResult = false
    }

    property var flightModesMenuItems: []
    property var ports: [5601, 5602, 5605, 5606, 5607]
    property var stream_names: ["Mark11", "Mark12", "Mark13", "Skeleton12", "Skeleton13"]

    function updateFlightModesMenu() {
        console.log(QGroundControl.settingsManager.videoSettings.udpPort.rawValue)
        var state = controller.status;
        var i;
        // Remove old menu items
        for (i = 0; i < flightModesMenuItems.length; i++) {
            flightModesMenu.removeItem(flightModesMenuItems[i])
        }
        flightModesMenuItems.length = 0
        // Add new items
        if(state == 1) return;
        for (i = 0; i < ports.length; i++) {
            var menuItem = flightModeMenuItemComponent.createObject(null, { "text": stream_names[i]})
            flightModesMenuItems.push(menuItem)
            flightModesMenu.insertItem(i, menuItem)
        }
    }

    function mainText()
    {
        updateFlightModesMenu();
        if(sshStatus == -1) return "Start ROS"
        if(sshStatus == 0) return "SSH successful!"
        if(sshStatus == 1) return "Start ROS (Connecting ...)"
        if(sshStatus == 2) return "SSH failed"
        return "Start ROS (Connection failed! Make sure your Network, SSH and VPN setup is working.)"
    }

    Component.onCompleted: _root.updateFlightModesMenu()

    Connections {
        target:                 QGroundControl.multiVehicleManager
        onActiveVehicleChanged: {_root.updateFlightModesMenu(); _root.setSSHStatus(currentVehicle ? _root.sshStatus : -1)}
    }

    MouseArea {
        id:                 mouseArea
        visible:            true
        anchors.leftMargin: mouseAreaLeftMargin
        anchors.fill:       parent
        onClicked:          flightModesMenu.popup((_root.width - flightModesMenu.width) / 2, _root.height)
    }
}
