/*
 * Copyright (C) 2026 CutefishOS Team.
 * Copyright (C) 2026 Project Aevis Contributors.
 *
 * Cutefish Photos Sidebar
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import FishUI 1.0 as FishUI
import Cutefish.Photos 1.0

Item {
    id: sideBar
    implicitWidth: 230

    property alias currentIndex: listView.currentIndex
    property alias model: listView.model

    signal folderSelected(string path, string typeId, string name)

    Rectangle {
        anchors.fill: parent
        color: FishUI.Theme.darkMode
               ? Qt.rgba(0.10, 0.10, 0.14, FishUI.Theme.blurEnabled ? 0.76 : 0.98)
               : Qt.rgba(0.95, 0.95, 0.97, FishUI.Theme.blurEnabled ? 0.72 : 0.98)

        Behavior on color {
            ColorAnimation { duration: 200 }
        }

        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: FishUI.Theme.glassBorderColor
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // macOS Window Controls Spacer
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: rootWindow.header.height
        }

        // Sidebar Header Title & Action
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            Layout.leftMargin: 16
            Layout.rightMargin: 12

            RowLayout {
                anchors.fill: parent
                spacing: 6

                Label {
                    text: qsTr("Library")
                    font.pixelSize: 16
                    font.bold: true
                    color: FishUI.Theme.textColor
                    Layout.fillWidth: true
                }

                IconButton {
                    width: 24
                    height: 24
                    source: "qrc:/images/cp-add.svg"
                    iconColor: FishUI.Theme.darkMode ? "#0A84FF" : "#007AFF"
                    tooltip: qsTr("Add Folder...")
                    onClicked: {
                        var chosen = PhotosApp.pickFolder(qsTr("Add Photos Folder"))
                        if (chosen && chosen.length > 0) {
                            folderModel.addCustomFolder(chosen)
                            var idx = folderModel.indexOfPath(chosen)
                            if (idx >= 0) {
                                listView.currentIndex = idx
                                var item = folderModel.data(folderModel.index(idx, 0), Qt.UserRole + 1)
                                sideBar.folderSelected(chosen, "folder", item)
                            }
                        }
                    }
                }
            }
        }

        // Scrollable Folder List
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: folderModel
            boundsBehavior: Flickable.StopAtBounds

            readonly property real sideInset: 10
            spacing: 2
            bottomMargin: 10

            FishUI.WheelHandler {
                target: listView
            }

            ScrollBar.vertical: ScrollBar {
                id: vbar
                active: listView.moving || listView.flicking
                policy: ScrollBar.AsNeeded
            }

            section.property: "section"
            section.criteria: ViewSection.FullString
            section.delegate: Item {
                width: listView.width
                height: 28

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: sideBar.sideInset + 6
                    anchors.rightMargin: sideBar.sideInset + 6
                    spacing: 4

                    Label {
                        text: section.toUpperCase()
                        font.pixelSize: 11
                        font.bold: true
                        color: FishUI.Theme.disabledTextColor
                        Layout.fillWidth: true
                        verticalAlignment: Text.AlignVCenter
                    }

                    IconButton {
                        visible: section === qsTr("All Folders")
                        width: 18
                        height: 18
                        iconWidth: 14
                        iconHeight: 14
                        source: "qrc:/images/cp-add.svg"
                        iconColor: FishUI.Theme.darkMode ? "#0A84FF" : "#007AFF"
                        tooltip: qsTr("Add Folder to Sidebar")
                        onClicked: {
                            var path = PhotosApp.pickFolder()
                            if (path && path.length > 0) {
                                folderModel.addCustomFolder(path)
                            }
                        }
                    }
                }
            }

            delegate: Item {
                id: _delegateItem
                width: listView.width
                height: 34

                readonly property bool isSelected: listView.currentIndex === index

                Rectangle {
                    anchors.fill: parent
                    anchors.leftMargin: listView.sideInset
                    anchors.rightMargin: listView.sideInset
                    radius: 8
                    color: {
                        if (isSelected)
                            return FishUI.Theme.highlightColor
                        if (_itemMouseArea.pressed)
                            return FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.14) : Qt.rgba(0, 0, 0, 0.10)
                        if (_itemMouseArea.containsMouse)
                            return FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.08) : Qt.rgba(0, 0, 0, 0.04)
                        return "transparent"
                    }

                    Behavior on color {
                        ColorAnimation { duration: 120 }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 8

                        FishUI.IconItem {
                            id: _folderIcon
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
                            Layout.alignment: Qt.AlignVCenter
                            source: model.icon ? model.icon : "qrc:/images/cp-sidebar-folder.svg"
                            color: isSelected ? "#FFFFFF" : (FishUI.Theme.darkMode ? "#0A84FF" : "#007AFF")
                        }

                        Label {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter
                            text: model.name
                            color: isSelected ? "#FFFFFF" : FishUI.Theme.textColor
                            font.weight: isSelected ? Font.DemiBold : Font.Normal
                            elide: Text.ElideRight
                        }

                        // Photo Count Badge
                        Rectangle {
                            visible: model.count > 0
                            Layout.alignment: Qt.AlignVCenter
                            implicitWidth: Math.max(18, countText.implicitWidth + 8)
                            implicitHeight: 18
                            radius: 9
                            color: isSelected ? Qt.rgba(1, 1, 1, 0.25)
                                              : (FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.10) : Qt.rgba(0, 0, 0, 0.06))

                            Text {
                                id: countText
                                anchors.centerIn: parent
                                text: model.count.toString()
                                font.pixelSize: 11
                                font.weight: Font.Medium
                                color: isSelected ? "#FFFFFF" : FishUI.Theme.disabledTextColor
                            }
                        }
                    }
                }

                MouseArea {
                    id: _itemMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    onClicked: {
                        if (mouse.button === Qt.LeftButton) {
                            listView.currentIndex = index
                            sideBar.folderSelected(model.path, model.typeId, model.name)
                        } else if (mouse.button === Qt.RightButton) {
                            contextMenu.popup()
                        }
                    }
                }

                FishUI.DesktopMenu {
                    id: contextMenu

                    FishUI.MenuItem {
                        text: qsTr("Open in File Manager")
                        visible: model.path && model.path.length > 0
                        onTriggered: {
                            PhotosApp.showInFileManager(model.path)
                        }
                    }

                    FishUI.MenuItem {
                        text: qsTr("Remove from Sidebar")
                        visible: model.isCustom
                        onTriggered: {
                            folderModel.removeCustomFolder(model.path)
                        }
                    }
                }
            }
        }
    }

    readonly property real sideInset: 10
}
