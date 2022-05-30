/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs  1.2

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.Controllers   1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.ScreenTools   1.0

SetupPage {
    id:             hookPage
    pageComponent:  pageComponent

    HookComponentController {
        id:             controller
    }

    Component {
        id: pageComponent

        Column {
            spacing: ScreenTools.defaultFontPixelHeight

            QGCLabel {
                anchors.left:   parent.left
                anchors.right:  parent.right
                wrapMode:       Text.WordWrap
                text:           qsTr("These controls can be used to move the hook and tune the closed position\n") +
                                qsTr("\"Backwards\" and \"Forwards\" buttons move the hook. The number controls how far the hook moves with one button press\n") +
                                qsTr("Once it is in the right position, use \"Set closed/open position\" to store this position as new closed/open position")
            }

            Row {
                spacing: ScreenTools.defaultFontPixelWidth
                QGCButton {
                    text: "Backwards"
                    onClicked: {controller.hookStep(-parseInt(numSteps.text));}
                }
                QGCTextField {
                    id:                 numSteps
                    maximumLength:      5
                    inputMethodHints:   Qt.ImhDigitsOnly
                    validator:          IntValidator {bottom: -9999; top: 9999;}
                    text:               qsTr("100")
                }
                QGCButton {
                    text: "Forwards"
                    onClicked: {controller.hookStep(parseInt(numSteps.text));}
                }
            }
            Row {
                spacing: ScreenTools.defaultFontPixelWidth
                QGCButton {
                    text: "Set closed position"
                    onClicked: {controller.hookReset(0)}
                }
                QGCButton {
                    text: "Set open position"
                    onClicked: {controller.hookReset(1)}
                }
            }
        } // Column
    } // Component
} // SetupPage
