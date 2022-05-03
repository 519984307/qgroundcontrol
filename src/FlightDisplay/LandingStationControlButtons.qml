/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick                      2.12
import QtQuick.Layouts              1.12

import QGroundControl               1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Vehicle       1.0
import QGroundControl.FactControls  1.0
import QGroundControl.Controls      1.0
import QGroundControl.Controllers   1.0
import QGroundControl.Palette       1.0

Rectangle {
    id:                 landingStationControls
    height:             buttonsAndStuff.height + ScreenTools.defaultFontPixelHeight
    width:              buttonsAndStuff.width + ScreenTools.defaultFontPixelWidth * 2
    radius:             ScreenTools.defaultFontPixelHeight / 2
    color:              qgcPal.window
    border.color:       qgcPal.text

    DeadMouseArea { anchors.fill: parent }

    property int _buttonWidth: ScreenTools.defaultFontPixelWidth * 12;
    property int _textWidth: ScreenTools.defaultFontPixelWidth * 12;

    property var _flyViewSettings: QGroundControl.settingsManager.flyViewSettings
    property var _currentVehicle:  QGroundControl.multiVehicleManager.activeVehicle

    visible: _currentVehicle && _flyViewSettings.landingStationControlsVisible.value


    LandingStationControlButtonsController {
        id: landingStationController
        speed: _flyViewSettings.landingStationSpeed.value
        timeout: _flyViewSettings.landingStationTimeout.value
    }

    GridLayout {
        anchors.centerIn:   parent
        id:               buttonsAndStuff
        Layout.alignment: Qt.AlignHCenter
        rows: 5
        columns: 3


        QGCLabel {
            Layout.alignment: Qt.AlignRight
            text: qsTr("Speed [%]")
        }
        FactTextField {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: _textWidth
            fact: _flyViewSettings.landingStationSpeed
            onEditingFinished: landingStationController.setSpeed(fact.value)
        }
        QGCButton {
            Layout.alignment: Qt.AlignHCenter
            text: "Deliver"
            Layout.preferredWidth: _buttonWidth
            onClicked: landingStationController.delivery()
        }

        QGCButton {
            Layout.alignment: Qt.AlignHCenter
            text: "Hook Close"
            Layout.preferredWidth: _buttonWidth
            onClicked: landingStationController.hookClose()
        }
        QGCButton {
            Layout.alignment: Qt.AlignHCenter
            text: "Up"
            Layout.preferredWidth: _buttonWidth
            onClicked: landingStationController.beltUp()
        }
        QGCButton {
            Layout.alignment: Qt.AlignHCenter
            text: "Hook Open"
            Layout.preferredWidth: _buttonWidth
            onClicked: landingStationController.hookOpen()
        }

        QGCButton {
            Layout.alignment: Qt.AlignRight
            text: "Left"
            Layout.preferredWidth: _buttonWidth
            onClicked: landingStationController.beltRotateLeft()
        }
        QGCButton {
            Layout.alignment: Qt.AlignHCenter
            text: "Stop"
            Layout.preferredWidth: _buttonWidth
            onClicked: landingStationController.beltStop()
        }
        QGCButton {
            Layout.alignment: Qt.AlignLeft
            text: "Right"
            Layout.preferredWidth: _buttonWidth
            onClicked: landingStationController.beltRotateRight()
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredWidth: _buttonWidth
        }
        QGCButton {
            Layout.alignment: Qt.AlignHCenter
            text: "Down"
            Layout.preferredWidth: _buttonWidth
            onClicked: landingStationController.beltDown()
        }
        Item {
            Layout.fillWidth: true
            Layout.preferredWidth: _buttonWidth
        }

        QGCLabel {
            Layout.alignment: Qt.AlignRight
            text: qsTr("Timeout [s]")
        }
        FactTextField {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: _textWidth
            fact: _flyViewSettings.landingStationTimeout
            onEditingFinished: landingStationController.setTimeout(fact.value)
        }
        Item {
            Layout.fillWidth: true
            Layout.preferredWidth: _buttonWidth
        }
    }
}
