/*
 * Copyright (C) 2026 CutefishOS Team.
 *
 * Inspector Info Panel for Cutefish Image Viewer
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import FishUI 1.0 as FishUI
import Cutefish.Photos 1.0

Item {
    id: infoPanel
    width: 280

    property string filePath: ""
    property var metadata: PhotosApp.getImageMetadata(filePath)

    onFilePathChanged: {
        metadata = PhotosApp.getImageMetadata(filePath)
    }

    signal closeRequested()

    Rectangle {
        anchors.fill: parent
        color: FishUI.Theme.darkMode ? Qt.rgba(0.14, 0.14, 0.16, 0.90) : Qt.rgba(0.96, 0.96, 0.97, 0.90)
        border.width: 1
        border.color: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.08) : Qt.rgba(0, 0, 0, 0.08)

        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        // Header Row
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("Information")
                font.pixelSize: 14
                font.bold: true
                color: FishUI.Theme.textColor
                Layout.fillWidth: true
            }

            IconButton {
                width: 22
                height: 22
                source: "qrc:/images/cp-go-next.svg"
                tooltip: qsTr("Close Panel")
                onClicked: infoPanel.closeRequested()
            }
        }

        // Thumbnail Preview Card
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 140
            radius: 8
            color: FishUI.Theme.darkMode ? Qt.rgba(0, 0, 0, 0.3) : Qt.rgba(0, 0, 0, 0.05)
            clip: true

            Image {
                anchors.centerIn: parent
                width: Math.min(parent.width - 8, implicitWidth)
                height: Math.min(parent.height - 8, implicitHeight)
                source: "image://photothumb/" + infoPanel.filePath + "?w=300&h=200"
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                smooth: true
            }
        }

        // Details List (Flickable)
        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentHeight: detailsColumn.implicitHeight
            clip: true

            Column {
                id: detailsColumn
                width: parent.width
                spacing: 10

                // File Name
                Column {
                    width: parent.width
                    spacing: 2
                    Label {
                        text: qsTr("FILE NAME")
                        font.pixelSize: 11
                        font.bold: true
                        color: FishUI.Theme.disabledTextColor
                    }
                    Label {
                        width: parent.width
                        text: infoPanel.metadata.fileName || ""
                        font.pixelSize: 12
                        font.weight: Font.Medium
                        color: FishUI.Theme.textColor
                        wrapMode: Text.WrapAnywhere
                    }
                }

                // Dimensions & Megapixels
                Column {
                    width: parent.width
                    spacing: 2
                    Label {
                        text: qsTr("DIMENSIONS")
                        font.pixelSize: 11
                        font.bold: true
                        color: FishUI.Theme.disabledTextColor
                    }
                    Label {
                        text: (infoPanel.metadata.dimensions || "") + (infoPanel.metadata.megapixels ? " (" + infoPanel.metadata.megapixels + ")" : "")
                        font.pixelSize: 12
                        color: FishUI.Theme.textColor
                    }
                }

                // Aspect Ratio
                Column {
                    width: parent.width
                    spacing: 2
                    visible: infoPanel.metadata.aspectRatio !== undefined && infoPanel.metadata.aspectRatio !== ""
                    Label {
                        text: qsTr("ASPECT RATIO")
                        font.pixelSize: 11
                        font.bold: true
                        color: FishUI.Theme.disabledTextColor
                    }
                    Label {
                        text: infoPanel.metadata.aspectRatio || ""
                        font.pixelSize: 12
                        color: FishUI.Theme.textColor
                    }
                }

                // File Size
                Column {
                    width: parent.width
                    spacing: 2
                    Label {
                        text: qsTr("FILE SIZE")
                        font.pixelSize: 11
                        font.bold: true
                        color: FishUI.Theme.disabledTextColor
                    }
                    Label {
                        text: infoPanel.metadata.fileSize || ""
                        font.pixelSize: 12
                        color: FishUI.Theme.textColor
                    }
                }

                // Format / Type
                Column {
                    width: parent.width
                    spacing: 2
                    Label {
                        text: qsTr("FORMAT")
                        font.pixelSize: 11
                        font.bold: true
                        color: FishUI.Theme.disabledTextColor
                    }
                    Label {
                        text: infoPanel.metadata.suffix || ""
                        font.pixelSize: 12
                        color: FishUI.Theme.textColor
                    }
                }

                // Modified Date
                Column {
                    width: parent.width
                    spacing: 2
                    Label {
                        text: qsTr("DATE MODIFIED")
                        font.pixelSize: 11
                        font.bold: true
                        color: FishUI.Theme.disabledTextColor
                    }
                    Label {
                        text: infoPanel.metadata.modified || ""
                        font.pixelSize: 12
                        color: FishUI.Theme.textColor
                    }
                }

                // File Location
                Column {
                    width: parent.width
                    spacing: 2
                    Label {
                        text: qsTr("LOCATION")
                        font.pixelSize: 11
                        font.bold: true
                        color: FishUI.Theme.disabledTextColor
                    }
                    Label {
                        width: parent.width
                        text: infoPanel.metadata.filePath || ""
                        font.pixelSize: 11
                        color: FishUI.Theme.disabledTextColor
                        wrapMode: Text.WrapAnywhere
                    }
                }

                // Quick Action Buttons
                RowLayout {
                    width: parent.width
                    spacing: 8

                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Copy Path")
                        onClicked: PhotosApp.copyPathToClipboard(infoPanel.filePath)
                    }

                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Reveal")
                        onClicked: PhotosApp.showInFileManager(infoPanel.filePath)
                    }
                }
            }
        }
    }
}
