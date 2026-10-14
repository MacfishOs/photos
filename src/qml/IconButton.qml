/*
 * Copyright (C) 2026 CutefishOS Team.
 *
 * Reusable Cutefish Icon Button
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import FishUI 1.0 as FishUI

Item {
    id: control
    width: 28
    height: 28

    property alias source: _image.source
    property alias iconWidth: _image.width
    property alias iconHeight: _image.height
    property alias iconColor: _image.color
    property string text: ""
    property string tooltip: ""
    property bool checkable: false
    property bool checked: false
    property color backgroundColor: "transparent"
    property color activeColor: FishUI.Theme.highlightColor
    property color hoveredColor: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.08)
    property color pressedColor: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.20) : Qt.rgba(0, 0, 0, 0.14)

    signal clicked()

    Rectangle {
        id: _background
        anchors.fill: parent
        radius: 6
        color: {
            if (control.checked)
                return control.activeColor
            if (!control.enabled)
                return "transparent"
            if (_mouseArea.pressed)
                return control.pressedColor
            if (_mouseArea.containsMouse)
                return control.hoveredColor
            return control.backgroundColor
        }

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
    }

    Row {
        anchors.centerIn: parent
        spacing: 4

        FishUI.IconItem {
            id: _image
            anchors.verticalCenter: parent.verticalCenter
            width: 18
            height: 18
            color: control.checked ? "#FFFFFF" : (control.enabled ? FishUI.Theme.textColor : FishUI.Theme.disabledTextColor)
            opacity: control.enabled ? 1.0 : 0.4
        }

        Label {
            id: _textLabel
            anchors.verticalCenter: parent.verticalCenter
            visible: control.text !== ""
            text: control.text
            color: control.checked ? "#FFFFFF" : (control.enabled ? FishUI.Theme.textColor : FishUI.Theme.disabledTextColor)
            font.weight: Font.Medium
            font.pixelSize: 12
            opacity: control.enabled ? 1.0 : 0.4
        }
    }

    MouseArea {
        id: _mouseArea
        anchors.fill: parent
        enabled: control.enabled
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            if (control.checkable) {
                control.checked = !control.checked
            }
            control.clicked()
        }
    }

    ToolTip {
        visible: control.tooltip !== "" && _mouseArea.containsMouse
        delay: 500
        text: control.tooltip
    }
}
