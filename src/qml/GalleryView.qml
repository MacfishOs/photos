/*
 * Copyright (C) 2026 CutefishOS Team.
 * Copyright (C) 2026 Project Aevis Contributors.
 *
 * Cutefish Photos Gallery View - High Performance & Fluid Grid
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import FishUI 1.0 as FishUI
import Cutefish.Photos 1.0

Item {
    id: galleryView
    clip: true

    property real gridSize: 180
    property int selectedIndex: -1

    signal photoDoubleClicked(int index)

    Rectangle {
        anchors.fill: parent
        color: FishUI.Theme.backgroundColor
    }

    // Main Photo Grid with High-Performance Buffering
    GridView {
        id: gridView
        anchors.fill: parent
        anchors.topMargin: rootWindow.header.height
        anchors.bottomMargin: 32
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        clip: true

        cellWidth: galleryView.gridSize + 12
        cellHeight: galleryView.gridSize + 36
        model: photoModel

        // Preload visible offscreen rows for instantaneous stutter-free scrolling
        cacheBuffer: Math.max(1200, gridView.height * 2.5)
        maximumFlickVelocity: 4000
        boundsBehavior: Flickable.StopAtBounds
        reuseItems: true
        focus: true

        Keys.onReturnPressed: {
            if (galleryView.selectedIndex >= 0)
                galleryView.photoDoubleClicked(galleryView.selectedIndex)
        }
        Keys.onEnterPressed: {
            if (galleryView.selectedIndex >= 0)
                galleryView.photoDoubleClicked(galleryView.selectedIndex)
        }

        FishUI.WheelHandler {
            target: gridView
        }

        ScrollBar.vertical: ScrollBar {
            id: vbar
            active: gridView.moving || gridView.flicking
            policy: ScrollBar.AsNeeded
        }

        delegate: Item {
            id: cardItem
            width: gridView.cellWidth - 10
            height: gridView.cellHeight - 10

            readonly property bool isSelected: galleryView.selectedIndex === index

            Rectangle {
                id: cardBackground
                anchors.fill: parent
                radius: 8
                color: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.04) : Qt.rgba(0, 0, 0, 0.03)
                border.width: isSelected ? 2 : (_mouseArea.containsMouse ? 1 : 0)
                border.color: isSelected ? FishUI.Theme.highlightColor
                                         : (FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.20) : Qt.rgba(0, 0, 0, 0.15))

                Behavior on border.color {
                    ColorAnimation { duration: 120 }
                }

                // Thumbnail Container
                Item {
                    id: imageArea
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: labelArea.top
                    anchors.margins: 4
                    clip: true

                    Rectangle {
                        anchors.fill: parent
                        radius: 6
                        color: FishUI.Theme.darkMode ? "#1C1C1E" : "#F0F0F2"

                        Image {
                            id: thumbImg
                            anchors.fill: parent
                            source: "image://photothumb/" + model.path + "?w=" + Math.round(galleryView.gridSize * 1.5) + "&h=" + Math.round(galleryView.gridSize * 1.5)
                            fillMode: Image.PreserveAspectCrop
                            asynchronous: true
                            smooth: true
                        }
                    }

                    // Favorite Star Badge on Card
                    IconButton {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 4
                        width: 24
                        height: 24
                        iconWidth: 16
                        iconHeight: 16
                        visible: model.isFavorite || _mouseArea.containsMouse
                        source: model.isFavorite ? "qrc:/images/cp-favorite-filled.svg" : "qrc:/images/cp-favorite.svg"
                        tooltip: model.isFavorite ? qsTr("Remove from Favorites") : qsTr("Add to Favorites")
                        onClicked: {
                            photoModel.toggleFavorite(index)
                        }
                    }
                }

                // Bottom Labels Area
                Item {
                    id: labelArea
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 28
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8

                    Column {
                        anchors.fill: parent
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 1

                        Label {
                            width: parent.width
                            text: model.name
                            font.pixelSize: 12
                            font.weight: isSelected ? Font.DemiBold : Font.Normal
                            color: FishUI.Theme.textColor
                            elide: Text.ElideRight
                        }

                        Label {
                            width: parent.width
                            text: (model.fileSize ? model.fileSize : "") + (model.dimensions ? " • " + model.dimensions : "")
                            font.pixelSize: 10
                            color: FishUI.Theme.disabledTextColor
                            elide: Text.ElideRight
                        }
                    }
                }
            }

            MouseArea {
                id: _mouseArea
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onClicked: function(mouse) {
                    galleryView.selectedIndex = index
                    if (mouse.button === Qt.RightButton) {
                        contextMenu.popup()
                    }
                }

                onDoubleClicked: {
                    galleryView.selectedIndex = index
                    galleryView.photoDoubleClicked(index)
                }
            }

            FishUI.DesktopMenu {
                id: contextMenu

                FishUI.MenuItem {
                    text: qsTr("Open in Viewer")
                    icon.source: "qrc:/images/cp-sidebar-photos.svg"
                    onTriggered: {
                        galleryView.photoDoubleClicked(index)
                    }
                }

                FishUI.MenuItem {
                    text: model.isFavorite ? qsTr("Remove from Favorites") : qsTr("Add to Favorites")
                    icon.source: model.isFavorite ? "qrc:/images/cp-favorite-filled.svg" : "qrc:/images/cp-favorite.svg"
                    onTriggered: {
                        photoModel.toggleFavorite(index)
                    }
                }

                FishUI.MenuSeparator {}

                FishUI.MenuItem {
                    text: qsTr("Copy Image")
                    icon.source: "qrc:/images/cp-copy.svg"
                    onTriggered: {
                        PhotosApp.copyImageToClipboard(model.path)
                    }
                }

                FishUI.MenuItem {
                    text: qsTr("Copy File Path")
                    icon.source: "qrc:/images/cp-copy.svg"
                    onTriggered: {
                        PhotosApp.copyPathToClipboard(model.path)
                    }
                }

                FishUI.MenuItem {
                    text: qsTr("Show in File Manager")
                    icon.source: "qrc:/images/cp-folder-open.svg"
                    onTriggered: {
                        PhotosApp.showInFileManager(model.path)
                    }
                }

                FishUI.MenuItem {
                    text: qsTr("Set as Desktop Wallpaper")
                    icon.source: "qrc:/images/cp-wallpaper.svg"
                    onTriggered: {
                        PhotosApp.setWallpaper(model.path)
                    }
                }

                FishUI.MenuSeparator {}

                FishUI.MenuItem {
                    text: qsTr("Move to Trash")
                    icon.source: "qrc:/images/cp-trash.svg"
                    onTriggered: {
                        rootWindow.confirmDeletePhoto(index)
                    }
                }
            }
        }
    }

    // Empty State Illustration
    Item {
        anchors.centerIn: parent
        width: 300
        height: 220
        visible: !photoModel.loading && photoModel.count === 0

        ColumnLayout {
            anchors.fill: parent
            spacing: 12

            Image {
                Layout.alignment: Qt.AlignHCenter
                width: 64
                height: 64
                source: "qrc:/images/cp-cutefish-photos.svg"
                opacity: 0.6
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("No Photos Found")
                font.pixelSize: 17
                font.bold: true
                color: FishUI.Theme.textColor
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("This folder does not contain any supported images.")
                font.pixelSize: 12
                color: FishUI.Theme.disabledTextColor
                horizontalAlignment: Text.AlignHCenter
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 8

                Button {
                    text: qsTr("Add Folder")
                    onClicked: {
                        var path = PhotosApp.pickFolder()
                        if (path && path.length > 0) {
                            folderModel.addCustomFolder(path)
                        }
                    }
                }

                Button {
                    visible: photoModel.currentPath && photoModel.currentPath.length > 0
                    text: qsTr("Open in File Manager")
                    onClicked: {
                        PhotosApp.showInFileManager(photoModel.currentPath)
                    }
                }
            }
        }
    }

    // Loading Indicator
    FishUI.BusyIndicator {
        anchors.centerIn: parent
        visible: photoModel.loading
        running: photoModel.loading
    }

    // Bottom Status Bar
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30
        color: FishUI.Theme.darkMode ? Qt.rgba(0.12, 0.12, 0.14, 0.6) : Qt.rgba(0.96, 0.96, 0.97, 0.6)
        border.width: 1
        border.color: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.06) : Qt.rgba(0, 0, 0, 0.06)

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 12

            Label {
                text: qsTr("%1 Photos").arg(photoModel.count)
                font.pixelSize: 11
                color: FishUI.Theme.disabledTextColor
                Layout.alignment: Qt.AlignVCenter
            }

            Item { Layout.fillWidth: true }

            // Thumbnail Size Zoom Slider
            RowLayout {
                spacing: 6
                Layout.alignment: Qt.AlignVCenter

                FishUI.IconItem {
                    width: 14
                    height: 14
                    source: "qrc:/images/cp-slider-small.svg"
                    color: FishUI.Theme.disabledTextColor
                }

                Slider {
                    id: sizeSlider
                    from: 120
                    to: 300
                    value: galleryView.gridSize
                    onMoved: {
                        galleryView.gridSize = value
                    }
                }

                FishUI.IconItem {
                    width: 18
                    height: 18
                    source: "qrc:/images/cp-slider-large.svg"
                    color: FishUI.Theme.disabledTextColor
                }
            }
        }
    }
}
