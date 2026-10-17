/*
 * Copyright (C) 2026 CutefishOS Team.
 * Copyright (C) 2026 Project Aevis Contributors.
 *
 * Bottom Filmstrip for Cutefish Image Viewer
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import FishUI 1.0 as FishUI
import Cutefish.Photos 1.0

Item {
    id: filmStrip
    height: 76

    property int currentIndex: 0
    property alias count: thumbnailView.count

    signal itemSelected(int index)

    Rectangle {
        anchors.fill: parent
        radius: 12
        color: FishUI.Theme.darkMode ? Qt.rgba(0.12, 0.12, 0.14, 0.85) : Qt.rgba(0.98, 0.98, 0.98, 0.85)
        border.width: 1
        border.color: FishUI.Theme.glassBorderColor

        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }

    ListView {
        id: thumbnailView
        anchors.fill: parent
        anchors.margins: 6
        orientation: ListView.Horizontal
        spacing: 8
        clip: true
        model: photoModel
        currentIndex: filmStrip.currentIndex

        onCurrentIndexChanged: {
            positionViewAtIndex(currentIndex, ListView.Center)
        }

        FishUI.WheelHandler {
            target: thumbnailView
        }

        delegate: Item {
            id: thumbDelegate
            width: 60
            height: 64

            readonly property bool isCurrent: filmStrip.currentIndex === index

            Rectangle {
                anchors.fill: parent
                radius: 6
                color: isCurrent ? FishUI.Theme.highlightColor : "transparent"
                border.width: isCurrent ? 2 : (thumbMouseArea.containsMouse ? 1 : 0)
                border.color: isCurrent ? FishUI.Theme.highlightColor
                                        : (FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.3) : Qt.rgba(0, 0, 0, 0.2))

                Image {
                    anchors.fill: parent
                    anchors.margins: isCurrent ? 2 : 1
                    source: "image://photothumb/" + model.path + "?w=120&h=120"
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    smooth: true
                    mipmap: true
                }
            }

            MouseArea {
                id: thumbMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    filmStrip.itemSelected(index)
                }
            }
        }
    }
}
