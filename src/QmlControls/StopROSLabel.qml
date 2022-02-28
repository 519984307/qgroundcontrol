import QtQuick                      2.12
import QtQuick.Controls             2.12
import QtQuick.Layouts              1.12

import QGroundControl                       1.0
import QGroundControl.FactSystem            1.0
import QGroundControl.FactControls          1.0
import QGroundControl.Controls              1.0
import QGroundControl.ScreenTools           1.0
import QGroundControl.MultiVehicleManager   1.0
import QGroundControl.Palette               1.0
import QGroundControl.Controllers           1.0
import QGroundControl.SettingsManager       1.0

// Label control which pops up a selection to stop ros or reboot the jetson
QGCLabel {
    id:     _root
    text:   qsTr("Stop ROS")

    property var    currentVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    property var    controller:             null
    property real   mouseAreaLeftMargin:    0
    property var    waitingForResult:       false

    Connections {
        target: controller
        function onStatusChanged(value) {
            // make sure we only react to results that we triggered ourselves
            if (waitingForResult && value == 0) {
                // SSH was succesull, we can reset our connection
                controller.resetConnection();
                waitingForResult = false;
            }
            waitingForResult = false;
        }
    }

    Component {
        id: youSureButtons

        Rectangle {
            width:  childColumn.width   + ScreenTools.defaultFontPixelWidth  * 3
            height: childColumn.height  + ScreenTools.defaultFontPixelHeight * 2
            radius: ScreenTools.defaultFontPixelHeight * 0.5
            color:  qgcPal.window
            border.color:   qgcPal.text

            Column{
                id: childColumn
                spacing:            ScreenTools.defaultFontPixelHeight * 0.5
                Layout.alignment: Qt.AlignHCenter || Qt.AlignVCenter
                anchors.centerIn: parent

                QGCLabel {
                    text: qsTr("This will interrupt the connection to the glider!\nAre you sure?")
                    horizontalAlignment: Text.AlignHCenter
                }

                Row {
                    id:                 buttonRow
                    spacing:            ScreenTools.defaultFontPixelHeight * 0.5
                    Layout.alignment:   Qt.AlignHCenter || Qt.AlignVCenter

                    QGCButton {
                        Layout.alignment: Qt.AlignLeft
                        text: qsTr("Stop ROS")
                        onClicked: {stopRos(); mainWindow.hideIndicatorPopup()}
                    }
                    QGCButton {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Cancel")
                        onClicked: mainWindow.hideIndicatorPopup()
                    }
                    QGCButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Reboot Jetson")
                        onClicked: {rebootJetson(); mainWindow.hideIndicatorPopup()}
                    }
                }
            }
        }
    }

    function stopRos() {
        if (controller.startCommand(qsTr("screen -S ros2 -p 0 -X stuff \"^C\"")) == 0) {
            waitingForResult = true;
        }
    }
    function rebootJetson() {
        if (controller.startCommand(qsTr("screen -S ros2 -p 0 -X stuff '^C'; mkdir test; sleep 5; sudo reboot now")) == 0) {
            waitingForResult = true;
        }
    }

    function maybeShowPopup() {
        if (currentVehicle.armed) {
            // don't even think about stopping ROS while flying
            return;
        } else {
            mainWindow.showIndicatorPopup(_root, youSureButtons);
        }
    }

    MouseArea {
        id:                 mouseArea
        visible:            true
        anchors.leftMargin: mouseAreaLeftMargin
        anchors.fill:       parent
        onClicked:          maybeShowPopup()
    }

}
