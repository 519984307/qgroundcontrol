/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick          2.11
import QtQuick.Layouts  1.11

import QGroundControl                       1.0
import QGroundControl.Controls              1.0
import QGroundControl.Controllers           1.0
import QGroundControl.MultiVehicleManager   1.0
import QGroundControl.ScreenTools           1.0
import QGroundControl.Palette               1.0

RowLayout {
    id:         _root
    spacing:    0

    property var    _activeVehicle:     QGroundControl.multiVehicleManager.activeVehicle
    property var    _vehicleInAir:      _activeVehicle ? _activeVehicle.flying || _activeVehicle.landing : false
    property bool   _vtolInFWDFlight:   _activeVehicle ? _activeVehicle.vtolInFwdFlight : false
    property bool   _armed:             _activeVehicle ? _activeVehicle.armed : false
    property real   _margins:           ScreenTools.defaultFontPixelWidth
    property real   _spacing:           ScreenTools.defaultFontPixelWidth / 2

    QGCLabel {
        id:             mainStatusLabel
        text:           mainStatusText()
        font.pointSize: _vehicleInAir ? ScreenTools.defaultFontPointSize : ScreenTools.largeFontPointSize

        property string _commLostText:      qsTr("Communication Lost")
        property string _readyToFlyText:    qsTr("Ready To Fly")
        property string _notReadyToFlyText: qsTr("Not Ready")
        property string _disconnectedText:  qsTr("Disconnected")
        property string _armedText:         qsTr("Armed")
        property string _flyingText:        qsTr("Flying")
        property string _landingText:       qsTr("Landing")

        function mainStatusText() {
            var statusText
            if (_activeVehicle) {
                if (_communicationLost) {
                    _mainStatusBGColor = "red"
                    return mainStatusLabel._commLostText
                }
                if (_activeVehicle.armed) {
                    _mainStatusBGColor = "green"
                    if (_activeVehicle.flying) {
                        return mainStatusLabel._flyingText
                    } else if (_activeVehicle.landing) {
                        return mainStatusLabel._landingText
                    } else {
                        return mainStatusLabel._armedText
                    }
                } else {
                    if (_activeVehicle.readyToFlyAvailable) {
                        if (_activeVehicle.readyToFly) {
                            _mainStatusBGColor = "green"
                            return mainStatusLabel._readyToFlyText
                        } else {
                            _mainStatusBGColor = "yellow"
                            return mainStatusLabel._notReadyToFlyText
                        }
                    } else {
                        // Best we can do is determine readiness based on AutoPilot component setup and health indicators from SYS_STATUS
                        if (_activeVehicle.allSensorsHealthy && _activeVehicle.autopilot.setupComplete) {
                            _mainStatusBGColor = "green"
                            return mainStatusLabel._readyToFlyText
                        } else {
                            _mainStatusBGColor = "yellow"
                            return mainStatusLabel._notReadyToFlyText
                        }
                    }
                }
            } else {
                _mainStatusBGColor = qgcPal.brandingPurple
                return mainStatusLabel._disconnectedText
            }
        }

        MouseArea {
            anchors.left:           parent.left
            anchors.right:          parent.right
            anchors.verticalCenter: parent.verticalCenter
            height:                 _root.height
            enabled:                _activeVehicle
            onClicked:              mainWindow.showIndicatorPopup(mainStatusLabel, sensorStatusInfoComponent)
        }
    }

    Item {
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * ScreenTools.largeFontPointRatio * 1.5
        height:                 1
    }

    QGCColoredImage {
        id:         flightModeIcon
        width:      ScreenTools.defaultFontPixelWidth * 2
        height:     ScreenTools.defaultFontPixelHeight * 0.75
        fillMode:   Image.PreserveAspectFit
        mipmap:     true
        color:      qgcPal.text
        source:     "/qmlimages/FlightModesComponentIcon.png"
        visible:    flightModeMenu.visible
    }

    Item {
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth / 2
        height:                 1
        visible:                flightModeMenu.visible
    }

    FlightModeMenu {
        id:                     flightModeMenu
        Layout.preferredHeight: _root.height
        verticalAlignment:      Text.AlignVCenter
        font.pointSize:         _vehicleInAir ?  ScreenTools.largeFontPointSize : ScreenTools.defaultFontPointSize
        mouseAreaLeftMargin:    -(flightModeMenu.x - flightModeIcon.x)
        visible:                _activeVehicle
    }

    Item {
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * ScreenTools.largeFontPointRatio * 1.5
        height:                 1
        visible:                vtolModeLabel.visible
    }

    QGCLabel {
        id:                     vtolModeLabel
        Layout.preferredHeight: _root.height
        verticalAlignment:      Text.AlignVCenter
        text:                   _vtolInFWDFlight ? qsTr("FW(vtol)") : qsTr("MR(vtol)")
        font.pointSize:         ScreenTools.largeFontPointSize
        visible:                _activeVehicle ? _activeVehicle.vtol && _vehicleInAir : false

        QGCMouseArea {
            anchors.fill:   parent
            onClicked:      mainWindow.showIndicatorPopup(vtolModeLabel, vtolTransitionComponent)
        }
    }

    Component {
        id: sensorStatusInfoComponent

        Rectangle {
            width:          flickable.width + (_margins * 2)
            height:         flickable.height + (_margins * 2)
            radius:         ScreenTools.defaultFontPixelHeight * 0.5
            color:          qgcPal.window
            border.color:   qgcPal.text

            QGCFlickable {
                id:                 flickable
                anchors.margins:    _margins
                anchors.top:        parent.top
                anchors.left:       parent.left
                width:              mainLayout.width
                height:             mainWindow.contentItem.height - (indicatorPopup.padding * 2) - (_margins * 2)
                flickableDirection: Flickable.VerticalFlick
                contentHeight:      mainLayout.height
                contentWidth:       mainLayout.width

                ColumnLayout {
                    id:         mainLayout
                    spacing:    _spacing

                    QGCButton {
                        Layout.alignment:   Qt.AlignHCenter
                        text:               _armed ?  qsTr("Disarm") : (forceArm ? qsTr("Force Arm") : qsTr("Arm"))

                        property bool forceArm: false

                        onPressAndHold: forceArm = true

                        onClicked: {
                            if (_armed) {
                                mainWindow.disarmVehicleRequest()
                            } else {
                                if (forceArm) {
                                    mainWindow.forceArmVehicleRequest()
                                } else {
                                    mainWindow.armVehicleRequest()
                                }
                            }
                            forceArm = false
                            mainWindow.hideIndicatorPopup()
                        }
                    }

                    QGCLabel {
                        Layout.alignment:   Qt.AlignHCenter
                        text:               qsTr("Sensor Status")
                    }

                    GridLayout {
                        rowSpacing:     _spacing
                        columnSpacing:  _spacing
                        rows:           _activeVehicle.sysStatusSensorInfo.sensorNames.length
                        flow:           GridLayout.TopToBottom

                        Repeater {
                            model: _activeVehicle.sysStatusSensorInfo.sensorNames

                            QGCLabel {
                                text: modelData
                            }
                        }

                        Repeater {
                            model: _activeVehicle.sysStatusSensorInfo.sensorStatus

                            QGCLabel {
                                text: modelData
                            }
                        }
                    }
                }
            }
        }
    }

    Item {
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * ScreenTools.largeFontPointRatio * 1.5
        height:                 1
        visible:                videoSourceIcon.visible
    }

    QGCColoredImage {
        id:         videoSourceIcon
        width:      ScreenTools.defaultFontPixelWidth * 2
        height:     ScreenTools.defaultFontPixelHeight * 0.75
        fillMode:   Image.PreserveAspectFit
        mipmap:     true
        color:      qgcPal.text
        source:     "/qmlimages/camera.svg"
        visible:    videoSourceMenu.visible
    }

    Item {
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth / 2
        height:                 1
        visible:                videoSourceMenu.visible
    }

    VideoSourceMenu {
        id:                     videoSourceMenu
        Layout.preferredHeight: _root.height
        verticalAlignment:      Text.AlignVCenter
        font.pointSize:         ScreenTools.defaultFontPointSize
        mouseAreaLeftMargin:    -(videoSourceMenu.x - videoSourceIcon.x)
        visible:                _activeVehicle
    }

    // Landing Station Indicator

    Item {
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * ScreenTools.largeFontPointRatio * 1.5
        height:                 1
        visible:                landingStationIcon.visible
    }

    QGCColoredImage {
        id:         landingStationIcon
        width:      ScreenTools.defaultFontPixelWidth * 2
        height:     ScreenTools.defaultFontPixelHeight * 0.75
        fillMode:   Image.PreserveAspectFit
        mipmap:     true
        color:      _activeVehicle ? (landingStationIndicator.currentVehicle.landingStationConnected.value? Qt.rgba(0,1,0,1):Qt.rgba(1,0,0,1)): Qt.rgba(1,1,1,1)
        source:     _activeVehicle ? (landingStationIndicator.currentVehicle.landingStationConnected.value ? "/qmlimages/GreenCheckmark.svg" : "/qmlimages/CircleX.svg") : ""
        visible:    _activeVehicle
    }

    Item {
        id:                     landingStationIndicatorItem
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth / 2
        height:                 1
        visible:                landingStationIndicator.visible
    }

    LandingStationIndicator {
        id:                     landingStationIndicator
        Layout.preferredHeight: _root.height
        verticalAlignment:      Text.AlignVCenter
        font.pointSize:         ScreenTools.defaultFontPointSize
        mouseAreaLeftMargin:    -(landingStationIndicator.x - landingStationIcon.x)
        visible:                _activeVehicle
        color:                  _activeVehicle ? (landingStationIndicator.currentVehicle.landingStationConnected.value? Qt.rgba(1,1,1,1):Qt.rgba(1,0,0,1)): Qt.rgba(1,1,1,1)
    }

    // Hook status indicator
    Item {
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * ScreenTools.largeFontPointRatio * 1.5
        height:                 1
        visible:                hookStatusIndicator.visible
    }

    function getHookColor() {
        if (_activeVehicle && _activeVehicle.hookStatus.value == 0) {
            // hook okay, retrun grean
            return Qt.rgba(0,1,0,1)
        } else if (_activeVehicle && _activeVehicle.hookStatus.value == 1) {
            // hook not okay, return red
            return Qt.rgba(1,0,0,1)
        }
        // no active vehicle or status unknown, return white
        return Qt.rgba(1,1,1,1)
    }

    function getHookIcon() {
        var status = "Unknown"
        var position = "Closed"
        if (_activeVehicle) {
            // check the hook status
            if (_activeVehicle.hookStatus.value == 0) {
                status = "Okay"
            } else if (_activeVehicle.hookStatus.value == 1) {
                status = "Failed"
            }

            // check the hook position
            if (_activeVehicle.hookPosition.value == 1) {
                // all other positions are considered closed
                position = "Open"
            }
        }
        return "/qmlimages/Hook" + position + status + ".svg"
    }

    QGCColoredImage {
        id:         hookStatusIndicator
        width:      ScreenTools.defaultFontPixelWidth * 5
        height:     ScreenTools.defaultFontPixelHeight * 2
        fillMode:   Image.PreserveAspectFit
        mipmap:     true
        color:      getHookColor()
        source:     getHookIcon()
        visible:    _activeVehicle
    }

    // Video FPS indicator
    Item {
        id:                     videoFPSItem
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth
        height:                 1
        visible:                videoFPS.visible
    }

    QGCLabel {
        id:                     videoFPS
        Layout.preferredHeight: _root.height
        verticalAlignment:      Text.AlignVCenter
        font.pointSize:         ScreenTools.defaultFontPointSize
        visible:                _activeVehicle
        text:                   "" + (_activeVehicle ? _activeVehicle.videoFPS.value : 0) + "FPS"
    }

    // Jetson temperature display
    function getTemperatureColor() {
        if (_activeVehicle && _activeVehicle.jetsonTemperature.value > 0 && _activeVehicle.jetsonTemperature.value <= 60) {
            // not too warm, return green
            return Qt.rgba(0,1,0,1)
        } else if (_activeVehicle && _activeVehicle.jetsonTemperature.value > 60 && _activeVehicle.jetsonTemperature.value <= 85) {
            // getting warmer, return orange
            return Qt.rgba(1,0.65,0,1)
        } else if (_activeVehicle && _activeVehicle.jetsonTemperature.value > 85) {
            // on fire, return red
            return Qt.rgba(1,0,0,1)
        }
        // no active vehicle or temperature unknown, return white
        return Qt.rgba(1,1,1,1)
    }

    Item {
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * ScreenTools.largeFontPointRatio * 1.5
        height:                 1
        visible:                jetsonTemperature.visible
    }

    QGCLabel {
        id:                     jetsonTemperature
        Layout.preferredHeight: _root.height
        verticalAlignment:      Text.AlignVCenter
        text:                   "Jetson: " + (_activeVehicle ? _activeVehicle.jetsonTemperature.value : 0) + "°C"
        font.pointSize:         ScreenTools.defaultFontPointSize
        visible:                _activeVehicle
        color:                  getTemperatureColor()
    }

    // detection quality indicator
    function getQualityColor() {
        var innovationMax = _activeVehicle ? _activeVehicle.innovationMax : 0.5
        var percentageMin = 100 * (1 - innovationMax)
        var percentageMiddle = (100 + percentageMin)/2

        if (_activeVehicle && _activeVehicle.tagDetectionQuality.value < percentageMin) {
            // everything below percentageMin % is really not good,
            // (innovation > PLD_INNOV_MX which is considered vision lost),
            // return red
            return Qt.rgba(1,0,0,1)
        } else if (_activeVehicle && _activeVehicle.tagDetectionQuality.value >= percentageMin && _activeVehicle.tagDetectionQuality.value < percentageMiddle) {
            // kinda okay (PLD_INNOV_MX/2 < innovation <= PLD_INNOV_MX), return orange
            return Qt.rgba(1,0.65,0,1)
        } else if (_activeVehicle && _activeVehicle.tagDetectionQuality.value >= percentageMiddle && _activeVehicle.tagDetectionQuality.value <= 100) {
            // good detections (low innovation), return green
            return Qt.rgba(0,1,0,1)
        }
        // no active vehicle
        return Qt.rgba(1,1,1,1)
    }

    Item {
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * ScreenTools.largeFontPointRatio * 1.5
        height:                 1
        visible:                tagDetectionQuality.visible
    }

    QGCLabel {
        id:                     tagDetectionQuality
        Layout.preferredHeight: _root.height
        verticalAlignment:      Text.AlignVCenter
        text:                   "Vision: " + (_activeVehicle && _activeVehicle.tagDetectionQuality.value <= 100 ? _activeVehicle.tagDetectionQuality.value : 0) + "%"
        font.pointSize:         ScreenTools.defaultFontPointSize
        visible:                _activeVehicle
        color:                  getQualityColor()
    }

    RosSSHController{
        id: sshController
    }

    // Start ROS Menu
    Item {
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * ScreenTools.largeFontPointRatio * 1.5
        height:                 1
    }

    QGCColoredImage {
        id:         rosIcon
        width:      ScreenTools.defaultFontPixelWidth * 2
        height:     ScreenTools.defaultFontPixelHeight * 0.75
        fillMode:   Image.PreserveAspectFit
        mipmap:     true
        color:      qgcPal.text
        source:     "/qmlimages/Hamburger.svg"
        visible:    !_activeVehicle
    }

    Item {
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth / 2
        height:                 1
        visible:                !_activeVehicle
    }

    StartROSMenu {
        id:                     rosMenu
        controller:             sshController
        Layout.preferredHeight: _root.height
        verticalAlignment:      Text.AlignVCenter
        font.pointSize:         ScreenTools.defaultFontPointSize
        mouseAreaLeftMargin:    -(rosMenu.x - rosIcon.x)
        visible:                !_activeVehicle
    }

    // Stop ROS controls
    Item {
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * ScreenTools.largeFontPointRatio * 1.5
        height:                 1
        visible:                stopRosIcon.visible
    }

    QGCColoredImage {
        id:         stopRosIcon
        width:      ScreenTools.defaultFontPixelWidth * 2
        height:     ScreenTools.defaultFontPixelHeight * 0.75
        fillMode:   Image.PreserveAspectFit
        mipmap:     true
        color:      qgcPal.text
        source:     "/qmlimages/Disconnect.svg"
        visible:    stopRosLabel.visible
        opacity:    stopRosLabel.opacity
    }

    Item {
        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth / 2
        height:                 1
        visible:                stopRosLabel.visible
    }

    StopROSLabel {
        id:                     stopRosLabel
        controller:             sshController
        Layout.preferredHeight: _root.height
        verticalAlignment:      Text.AlignVCenter
        font.pointSize:         ScreenTools.defaultFontPointSize
        mouseAreaLeftMargin:    -(stopRosLabel.x - stopRosIcon.x)
        visible:                _activeVehicle && !_communicationLost
        opacity:                _armed ? 0.3 : 1
    }

    Component {
        id: vtolTransitionComponent

        Rectangle {
            width:          mainLayout.width   + (_margins * 2)
            height:         mainLayout.height  + (_margins * 2)
            radius:         ScreenTools.defaultFontPixelHeight * 0.5
            color:          qgcPal.window
            border.color:   qgcPal.text

            QGCButton {
                id:                 mainLayout
                anchors.margins:    _margins
                anchors.top:        parent.top
                anchors.left:       parent.left
                text:               _vtolInFWDFlight ? qsTr("Transition to Multi-Rotor") : qsTr("Transition to Fixed Wing")

                onClicked: {
                    if (_vtolInFWDFlight) {
                        mainWindow.vtolTransitionToMRFlightRequest()
                    } else {
                        mainWindow.vtolTransitionToFwdFlightRequest()
                    }
                    mainWindow.hideIndicatorPopup()
                }
            }
        }
    }
}

