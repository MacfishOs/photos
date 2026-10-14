/*
 * Copyright (C) 2026 CutefishOS Team.
 *
 * Cutefish Photos Application
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.12
import FishUI 1.0 as FishUI
import Cutefish.Photos 1.0

FishUI.Window {
    id: rootWindow
    title: viewMode === "viewer" && currentPhotoPath !== "" 
           ? (PhotosApp.fileName(currentPhotoPath) + " - " + qsTr("Photos"))
           : (currentFolderName !== "" ? (currentFolderName + " - " + qsTr("Photos")) : qsTr("Photos"))
    visible: true
    width: 1060
    height: 700

    minimumWidth: 860
    minimumHeight: 560

    background.color: "transparent"
    background.opacity: 1
    header.height: 40
    contentTopMargin: 0

    property string viewMode: "gallery" // "gallery" or "viewer"
    property int currentPhotoIndex: -1
    property string currentPhotoPath: ""
    property string currentFolderName: qsTr("All Photos")

    // Backend Models
    FolderModel {
        id: folderModel
    }

    PhotoModel {
        id: photoModel
    }

    // Toast Notification helper
    function showToast(message, isSuccess) {
        toastText.text = message
        toastIcon.source = isSuccess ? "qrc:/images/cp-check.svg" : "qrc:/images/cp-warning.svg"
        toastIcon.color = isSuccess ? "#34C759" : "#FF9500"
        toastPopup.open()
        toastTimer.restart()
    }

    Connections {
        target: PhotosApp
        function onActionFeedback(message, success) {
            rootWindow.showToast(message, success)
        }
    }

    function openViewer(index) {
        if (index >= 0 && index < photoModel.count) {
            currentPhotoIndex = index
            currentPhotoPath = photoModel.pathOfIndex(index)
            viewMode = "viewer"
        }
    }

    function backToGallery() {
        viewMode = "gallery"
        if (currentPhotoIndex >= 0) {
            galleryView.selectedIndex = currentPhotoIndex
        }
    }

    function nextPhoto() {
        if (currentPhotoIndex < photoModel.count - 1) {
            openViewer(currentPhotoIndex + 1)
        }
    }

    function prevPhoto() {
        if (currentPhotoIndex > 0) {
            openViewer(currentPhotoIndex - 1)
        }
    }

    function confirmDeletePhoto(index) {
        if (index >= 0 && index < photoModel.count) {
            var fileName = photoModel.get(index).name
            deleteConfirmDialog.targetIndex = index
            deleteConfirmDialog.targetFileName = fileName
            deleteConfirmDialog.open()
        }
    }

    Component.onCompleted: {
        if (initialImagePath && initialImagePath.length > 0) {
            var parentDir = PhotosApp.parentDirectory(initialImagePath)
            folderModel.addCustomFolder(parentDir)
            var fIdx = folderModel.indexOfPath(parentDir)
            if (fIdx >= 0) {
                sideBar.currentIndex = fIdx
            }
            photoModel.load(parentDir, "folder")
            var pIdx = photoModel.indexOfPath(initialImagePath)
            if (pIdx >= 0) {
                openViewer(pIdx)
            } else if (photoModel.count > 0) {
                openViewer(0)
            }
        }
    }

    // Unified Header Bar Controls
    headerItem: Item {
        id: customHeader
        anchors.fill: parent

        // -------------------------------------------------------------
        // GALLERY MODE HEADER
        // -------------------------------------------------------------
        Item {
            id: galleryHeader
            anchors.fill: parent
            visible: rootWindow.viewMode === "gallery"

            RowLayout {
                anchors.fill: parent
                spacing: 0

                // Area above sidebar (matching sidebar width minus window controls container)
                Item {
                    Layout.preferredWidth: Math.max(10, sideBar.width - 88)
                    Layout.fillHeight: true

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 6
                        anchors.rightMargin: 10

                        Label {
                            text: qsTr("Photos")
                            font.pixelSize: 14
                            font.bold: true
                            color: FishUI.Theme.textColor
                            Layout.alignment: Qt.AlignVCenter
                        }

                        Item { Layout.fillWidth: true }
                    }
                }

                // 1px Vertical Divider aligning with Sidebar boundary
                Rectangle {
                    Layout.preferredWidth: 1
                    Layout.fillHeight: true
                    color: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.08) : Qt.rgba(0, 0, 0, 0.08)
                }

                // Content Area Header Toolbar
                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.leftMargin: 16
                    Layout.rightMargin: 14
                    spacing: 10

                    // Current Folder Title & Item Count Pill
                    RowLayout {
                        spacing: 8
                        Layout.alignment: Qt.AlignVCenter

                        Label {
                            text: rootWindow.currentFolderName
                            font.pixelSize: 14
                            font.bold: true
                            color: FishUI.Theme.textColor
                            elide: Text.ElideRight
                        }

                        Rectangle {
                            implicitWidth: countBadgeText.implicitWidth + 12
                            implicitHeight: 20
                            radius: 10
                            color: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.06)

                            Text {
                                id: countBadgeText
                                anchors.centerIn: parent
                                text: photoModel.count.toString()
                                font.pixelSize: 11
                                font.weight: Font.Medium
                                color: FishUI.Theme.disabledTextColor
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // Search Filter Box
                    TextField {
                        id: searchField
                        Layout.preferredWidth: 200
                        Layout.preferredHeight: 28
                        Layout.alignment: Qt.AlignVCenter
                        leftPadding: 26
                        placeholderText: qsTr("Search photos...")
                        onTextChanged: {
                            photoModel.setFilterText(text)
                        }

                        FishUI.IconItem {
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            width: 13
                            height: 13
                            source: "qrc:/images/cp-search.svg"
                            color: FishUI.Theme.disabledTextColor
                        }
                    }

                    // Sort Menu Button
                    IconButton {
                        width: 28
                        height: 28
                        Layout.alignment: Qt.AlignVCenter
                        source: "qrc:/images/cp-sort.svg"
                        tooltip: qsTr("Sort Photos")
                        onClicked: sortMenu.popup()

                        FishUI.DesktopMenu {
                            id: sortMenu

                            FishUI.MenuItem {
                                text: qsTr("Sort by Date (Newest)")
                                checkable: true
                                checked: photoModel.sortRole === 0 && photoModel.sortDescending
                                onTriggered: {
                                    photoModel.setSortRole(0)
                                    photoModel.setSortDescending(true)
                                }
                            }

                            FishUI.MenuItem {
                                text: qsTr("Sort by Date (Oldest)")
                                checkable: true
                                checked: photoModel.sortRole === 0 && !photoModel.sortDescending
                                onTriggered: {
                                    photoModel.setSortRole(0)
                                    photoModel.setSortDescending(false)
                                }
                            }

                            FishUI.MenuSeparator {}

                            FishUI.MenuItem {
                                text: qsTr("Sort by Name (A-Z)")
                                checkable: true
                                checked: photoModel.sortRole === 1 && !photoModel.sortDescending
                                onTriggered: {
                                    photoModel.setSortRole(1)
                                    photoModel.setSortDescending(false)
                                }
                            }

                            FishUI.MenuItem {
                                text: qsTr("Sort by Name (Z-A)")
                                checkable: true
                                checked: photoModel.sortRole === 1 && photoModel.sortDescending
                                onTriggered: {
                                    photoModel.setSortRole(1)
                                    photoModel.setSortDescending(true)
                                }
                            }

                            FishUI.MenuSeparator {}

                            FishUI.MenuItem {
                                text: qsTr("Sort by Size (Largest)")
                                checkable: true
                                checked: photoModel.sortRole === 2 && photoModel.sortDescending
                                onTriggered: {
                                    photoModel.setSortRole(2)
                                    photoModel.setSortDescending(true)
                                }
                            }

                            FishUI.MenuItem {
                                text: qsTr("Sort by Size (Smallest)")
                                checkable: true
                                checked: photoModel.sortRole === 2 && !photoModel.sortDescending
                                onTriggered: {
                                    photoModel.setSortRole(2)
                                    photoModel.setSortDescending(false)
                                }
                            }
                        }
                    }

                    // Refresh Button
                    IconButton {
                        width: 28
                        height: 28
                        Layout.alignment: Qt.AlignVCenter
                        source: "qrc:/images/cp-refresh.svg"
                        tooltip: qsTr("Refresh")
                        onClicked: {
                            folderModel.refresh()
                            photoModel.refresh()
                        }
                    }

                    // Open Folder in File Manager
                    IconButton {
                        width: 28
                        height: 28
                        Layout.alignment: Qt.AlignVCenter
                        visible: photoModel.currentPath !== ""
                        source: "qrc:/images/cp-folder-open.svg"
                        tooltip: qsTr("Open Folder in File Manager")
                        onClicked: {
                            PhotosApp.showInFileManager(photoModel.currentPath)
                        }
                    }
                }
            }
        }

        // -------------------------------------------------------------
        // IMAGE VIEWER MODE HEADER
        // -------------------------------------------------------------
        Item {
            id: viewerHeader
            anchors.fill: parent
            visible: rootWindow.viewMode === "viewer"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 6
                anchors.rightMargin: 14
                spacing: 8

                // Dedicated Back Button right next to macOS traffic lights!
                Rectangle {
                    id: backPillButton
                    Layout.preferredHeight: 28
                    Layout.preferredWidth: backButtonLayout.implicitWidth + 16
                    Layout.alignment: Qt.AlignVCenter
                    radius: 6
                    color: backMouseArea.pressed ? (FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.20) : Qt.rgba(0, 0, 0, 0.14))
                                                 : (backMouseArea.containsMouse ? (FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.08))
                                                                                : (FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.06) : Qt.rgba(0, 0, 0, 0.04)))

                    Behavior on color {
                        ColorAnimation { duration: 120 }
                    }

                    RowLayout {
                        id: backButtonLayout
                        anchors.centerIn: parent
                        spacing: 4

                        FishUI.IconItem {
                            width: 16
                            height: 16
                            source: "qrc:/images/cp-go-previous.svg"
                            color: FishUI.Theme.textColor
                        }

                        Label {
                            text: qsTr("Photos")
                            font.weight: Font.DemiBold
                            font.pixelSize: 12
                            color: FishUI.Theme.textColor
                        }
                    }

                    MouseArea {
                        id: backMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: rootWindow.backToGallery()
                    }

                    ToolTip {
                        visible: backMouseArea.containsMouse
                        text: qsTr("Back to Gallery (Esc)")
                    }
                }

                // Center Title & Counter
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 1

                        RowLayout {
                            Layout.alignment: Qt.AlignHCenter
                            spacing: 6

                            Label {
                                text: PhotosApp.fileName(rootWindow.currentPhotoPath)
                                font.pixelSize: 13
                                font.bold: true
                                color: FishUI.Theme.textColor
                                elide: Text.ElideMiddle
                                maximumLineCount: 1
                            }

                            Rectangle {
                                visible: photoModel.count > 0
                                implicitWidth: posBadgeText.implicitWidth + 8
                                implicitHeight: 16
                                radius: 8
                                color: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.06)

                                Text {
                                    id: posBadgeText
                                    anchors.centerIn: parent
                                    text: qsTr("%1 of %2").arg(rootWindow.currentPhotoIndex + 1).arg(photoModel.count)
                                    font.pixelSize: 10
                                    font.weight: Font.Medium
                                    color: FishUI.Theme.disabledTextColor
                                }
                            }
                        }
                    }
                }

                // Right Actions: ONLY Share and Set Wallpaper as requested
                // 1. Share Action Pill Button
                Rectangle {
                    id: sharePillButton
                    Layout.preferredHeight: 28
                    Layout.preferredWidth: shareButtonLayout.implicitWidth + 16
                    Layout.alignment: Qt.AlignVCenter
                    radius: 6
                    color: shareMouseArea.pressed ? (FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.20) : Qt.rgba(0, 0, 0, 0.14))
                                                  : (shareMouseArea.containsMouse ? (FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.08))
                                                                                 : (FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.06) : Qt.rgba(0, 0, 0, 0.04)))

                    Behavior on color {
                        ColorAnimation { duration: 120 }
                    }

                    RowLayout {
                        id: shareButtonLayout
                        anchors.centerIn: parent
                        spacing: 5

                        FishUI.IconItem {
                            width: 16
                            height: 16
                            source: "qrc:/images/cp-share.svg"
                            color: FishUI.Theme.textColor
                        }

                        Label {
                            text: qsTr("Share")
                            font.weight: Font.Medium
                            font.pixelSize: 12
                            color: FishUI.Theme.textColor
                        }
                    }

                    MouseArea {
                        id: shareMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            shareMenu.filePath = rootWindow.currentPhotoPath
                            shareMenu.popup()
                        }
                    }

                    ToolTip {
                        visible: shareMouseArea.containsMouse
                        text: qsTr("Share Options")
                    }
                }

                // 2. Set as Desktop Wallpaper Pill Button
                Rectangle {
                    id: wallpaperPillButton
                    Layout.preferredHeight: 28
                    Layout.preferredWidth: wallpaperLayout.implicitWidth + 16
                    Layout.alignment: Qt.AlignVCenter
                    radius: 6
                    color: wpMouseArea.pressed ? (FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.20) : Qt.rgba(0, 0, 0, 0.14))
                                               : (wpMouseArea.containsMouse ? (FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.08))
                                                                              : (FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.06) : Qt.rgba(0, 0, 0, 0.04)))

                    Behavior on color {
                        ColorAnimation { duration: 120 }
                    }

                    RowLayout {
                        id: wallpaperLayout
                        anchors.centerIn: parent
                        spacing: 5

                        FishUI.IconItem {
                            width: 16
                            height: 16
                            source: "qrc:/images/cp-wallpaper.svg"
                            color: FishUI.Theme.textColor
                        }

                        Label {
                            text: qsTr("Wallpaper")
                            font.weight: Font.Medium
                            font.pixelSize: 12
                            color: FishUI.Theme.textColor
                        }
                    }

                    MouseArea {
                        id: wpMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            PhotosApp.setWallpaper(rootWindow.currentPhotoPath)
                        }
                    }

                    ToolTip {
                        visible: wpMouseArea.containsMouse
                        text: qsTr("Set as Desktop Wallpaper")
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------
    // MAIN CONTENT AREA
    // -----------------------------------------------------------------
    Item {
        anchors.fill: parent

        // View 1: Main Gallery with SideBar
        RowLayout {
            id: galleryLayout
            anchors.fill: parent
            spacing: 0
            visible: rootWindow.viewMode === "gallery"

            // Left Sidebar
            SideBar {
                id: sideBar
                Layout.preferredWidth: 230
                Layout.fillHeight: true

                onFolderSelected: function(path, typeId, name) {
                    rootWindow.currentFolderName = name
                    photoModel.load(path, typeId)
                }
            }

            // 1px Vertical Divider
            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                color: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.08) : Qt.rgba(0, 0, 0, 0.08)
                z: 5
            }

            // Photo Grid Gallery
            GalleryView {
                id: galleryView
                Layout.fillWidth: true
                Layout.fillHeight: true

                onPhotoDoubleClicked: function(index) {
                    rootWindow.openViewer(index)
                }
            }
        }

        // View 2: Built-in Image Viewer
        ImageViewer {
            id: imageViewer
            anchors.fill: parent
            anchors.topMargin: rootWindow.header.height
            visible: rootWindow.viewMode === "viewer"

            onBackRequested: rootWindow.backToGallery()
            onNextRequested: rootWindow.nextPhoto()
            onPrevRequested: rootWindow.prevPhoto()
        }
    }

    // Share Menu instance
    ShareMenu {
        id: shareMenu
    }

    // Confirmation Dialog for Deletion
    FishUI.ConfirmDialog {
        id: deleteConfirmDialog
        property int targetIndex: -1
        property string targetFileName: ""

        title: qsTr("Move to Trash")
        text: qsTr("Are you sure you want to move \"%1\" to Trash?").arg(targetFileName)

        onAccepted: {
            if (targetIndex >= 0) {
                photoModel.deletePhoto(targetIndex)
                if (rootWindow.viewMode === "viewer") {
                    if (photoModel.count === 0) {
                        rootWindow.backToGallery()
                    } else if (rootWindow.currentPhotoIndex >= photoModel.count) {
                        rootWindow.openViewer(photoModel.count - 1)
                    } else {
                        rootWindow.openViewer(rootWindow.currentPhotoIndex)
                    }
                }
            }
        }
    }

    // Bottom Toast Notification Pill
    Popup {
        id: toastPopup
        anchors.centerIn: parent
        y: parent.height - height - 40
        closePolicy: Popup.NoAutoClose
        modal: false
        padding: 0

        background: Rectangle {
            implicitWidth: toastContent.implicitWidth + 32
            implicitHeight: 38
            radius: 19
            color: FishUI.Theme.darkMode ? Qt.rgba(0.18, 0.18, 0.20, 0.95) : Qt.rgba(0.98, 0.98, 0.98, 0.95)
            border.width: 1
            border.color: FishUI.Theme.glassBorderColor

            RowLayout {
                id: toastContent
                anchors.centerIn: parent
                spacing: 8

                FishUI.IconItem {
                    id: toastIcon
                    width: 18
                    height: 18
                    color: FishUI.Theme.highlightColor
                }

                Label {
                    id: toastText
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    color: FishUI.Theme.textColor
                }
            }
        }

        Timer {
            id: toastTimer
            interval: 2500
            onTriggered: toastPopup.close()
        }
    }
}
