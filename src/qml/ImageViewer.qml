/*
 * Copyright (C) 2026 CutefishOS Team.
 * Copyright (C) 2026 Project Aevis Contributors.
 *
 * Ultra-Fluid Cutefish Built-in Image Viewer
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import FishUI 1.0 as FishUI
import Cutefish.Photos 1.0

Item {
    id: viewer
    clip: true

    property int currentIndex: rootWindow.currentPhotoIndex
    property var currentPhoto: (currentIndex >= 0 && currentIndex < photoModel.count) ? photoModel.get(currentIndex) : null
    property string currentPath: currentPhoto && currentPhoto.path ? currentPhoto.path : rootWindow.currentPhotoPath
    property url currentUrl: currentPhoto && currentPhoto.url ? currentPhoto.url : (rootWindow.currentPhotoPath !== "" ? PhotosApp.localFileUrl(rootWindow.currentPhotoPath) : "")

    property real zoomFactor: 1.0
    property real minZoom: 0.1
    property real maxZoom: 10.0
    property real currentRotation: 0
    property bool isFlipped: false
    property bool infoPanelVisible: false
    property bool filmStripVisible: true

    signal backRequested()
    signal nextRequested()
    signal prevRequested()

    function resetView() {
        zoomFactor = 1.0
        currentRotation = 0
        isFlipped = false
        imageContainer.x = (viewport.width - imageContainer.width) / 2
        imageContainer.y = (viewport.height - imageContainer.height) / 2
    }

    function fitToScreen() {
        if (mainImage.implicitWidth > 0 && mainImage.implicitHeight > 0) {
            var availW = viewport.width - 40
            var availH = viewport.height - (filmStripVisible ? 100 : 40)
            var scaleX = availW / mainImage.implicitWidth
            var scaleY = availH / mainImage.implicitHeight
            zoomFactor = Math.min(1.0, Math.min(scaleX, scaleY))
            imageContainer.x = (viewport.width - imageContainer.width) / 2
            imageContainer.y = (viewport.height - imageContainer.height) / 2
        }
    }

    function setZoom(factor) {
        zoomFactor = Math.max(minZoom, Math.min(maxZoom, factor))
    }

    function zoomIn() {
        setZoom(zoomFactor * 1.25)
    }

    function zoomOut() {
        setZoom(zoomFactor / 1.25)
    }

    function rotateClockwise() {
        currentRotation = (currentRotation + 90) % 360
    }

    function rotateCounterClockwise() {
        currentRotation = (currentRotation - 90 + 360) % 360
    }

    function toggleFlip() {
        isFlipped = !isFlipped
    }

    onCurrentIndexChanged: {
        resetView()
        fitToScreen()
        slideTransition.restart()
    }

    Component.onCompleted: {
        fitToScreen()
    }

    // -----------------------------------------------------------------
    // High-Performance Adjacent Image Preloading
    // Preloads previous and next images into memory for instant navigation
    // -----------------------------------------------------------------
    Image {
        id: preloadNext
        visible: false
        width: 1
        height: 1
        sourceSize.width: 1920
        sourceSize.height: 1080
        asynchronous: true
        cache: true
        source: (viewer.currentIndex >= 0 && viewer.currentIndex + 1 < photoModel.count) ? ("file://" + photoModel.pathOfIndex(viewer.currentIndex + 1)) : ""
    }

    Image {
        id: preloadPrev
        visible: false
        width: 1
        height: 1
        sourceSize.width: 1920
        sourceSize.height: 1080
        asynchronous: true
        cache: true
        source: (viewer.currentIndex > 0) ? ("file://" + photoModel.pathOfIndex(viewer.currentIndex - 1)) : ""
    }

    // Background Canvas
    Rectangle {
        anchors.fill: parent
        color: FishUI.Theme.backgroundColor

        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }

    // Main Interactive Viewport
    Item {
        id: viewport
        anchors.fill: parent
        anchors.rightMargin: infoPanelVisible ? infoPanel.width : 0
        clip: true

        Behavior on anchors.rightMargin {
            NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
        }

        MouseArea {
            id: viewportMouseArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
            drag.target: zoomFactor > 1.05 ? imageContainer : null
            drag.axis: Drag.XAndYAxis

            onDoubleClicked: {
                if (Math.abs(viewer.zoomFactor - 1.0) < 0.05) {
                    viewer.fitToScreen()
                } else {
                    viewer.setZoom(1.0)
                    imageContainer.x = (viewport.width - imageContainer.width) / 2
                    imageContainer.y = (viewport.height - imageContainer.height) / 2
                }
            }

            onClicked: function(mouse) {
                if (mouse.button === Qt.RightButton) {
                    viewerContextMenu.popup()
                }
            }

            onWheel: function(wheel) {
                var factor = wheel.angleDelta.y > 0 ? 1.15 : (1.0 / 1.15)
                var newZoom = Math.max(viewer.minZoom, Math.min(viewer.maxZoom, viewer.zoomFactor * factor))
                
                var mouseX = wheel.x
                var mouseY = wheel.y
                var relX = (mouseX - imageContainer.x) / viewer.zoomFactor
                var relY = (mouseY - imageContainer.y) / viewer.zoomFactor

                viewer.zoomFactor = newZoom
                imageContainer.x = mouseX - relX * newZoom
                imageContainer.y = mouseY - relY * newZoom
            }
        }

        Item {
            id: imageContainer
            width: mainImage.implicitWidth * viewer.zoomFactor
            height: mainImage.implicitHeight * viewer.zoomFactor
            x: (viewport.width - width) / 2
            y: (viewport.height - height) / 2

            // Low-res thumbnail instant placeholder (prevents blank screen during decode)
            Image {
                id: thumbPlaceholder
                anchors.fill: parent
                source: viewer.currentPath !== "" ? ("image://photothumb/" + viewer.currentPath + "?w=600&h=600") : ""
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                smooth: true
                visible: mainImage.status !== Image.Ready
                opacity: mainImage.status === Image.Ready ? 0.0 : 1.0

                Behavior on opacity {
                    NumberAnimation { duration: 150 }
                }
            }

            // High-Resolution Full Image
            Image {
                id: mainImage
                anchors.fill: parent
                source: viewer.currentUrl
                asynchronous: true
                cache: true
                smooth: true
                mipmap: true
                fillMode: Image.PreserveAspectFit

                rotation: viewer.currentRotation
                transform: Scale {
                    origin.x: mainImage.width / 2
                    origin.y: mainImage.height / 2
                    xScale: viewer.isFlipped ? -1 : 1
                }

                Behavior on rotation {
                    NumberAnimation { duration: 180; easing.type: Easing.OutQuad }
                }

                onStatusChanged: {
                    if (status === Image.Ready) {
                        viewer.fitToScreen()
                    }
                }
            }
        }

        // Fluid navigation slide transition
        SequentialAnimation {
            id: slideTransition
            NumberAnimation {
                target: imageContainer
                property: "opacity"
                from: 0.7
                to: 1.0
                duration: 160
                easing.type: Easing.OutCubic
            }
        }

        // Overlay Navigation Arrows: Previous & Next
        // Left Floating Arrow
        Rectangle {
            id: prevButton
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            width: 44
            height: 44
            radius: 22
            color: prevMouse.containsMouse ? (FishUI.Theme.darkMode ? Qt.rgba(0.2, 0.2, 0.22, 0.9) : Qt.rgba(1, 1, 1, 0.95))
                                           : (FishUI.Theme.darkMode ? Qt.rgba(0.1, 0.1, 0.12, 0.6) : Qt.rgba(1, 1, 1, 0.75))
            border.width: 1
            border.color: FishUI.Theme.glassBorderColor
            visible: viewer.currentIndex > 0
            opacity: viewportMouseArea.containsMouse || prevMouse.containsMouse ? 1.0 : 0.0

            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }

            FishUI.IconItem {
                anchors.centerIn: parent
                width: 20
                height: 20
                source: "qrc:/images/cp-go-previous.svg"
                color: FishUI.Theme.textColor
            }

            MouseArea {
                id: prevMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: viewer.prevRequested()
            }
        }

        // Right Floating Arrow
        Rectangle {
            id: nextButton
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            width: 44
            height: 44
            radius: 22
            color: nextMouse.containsMouse ? (FishUI.Theme.darkMode ? Qt.rgba(0.2, 0.2, 0.22, 0.9) : Qt.rgba(1, 1, 1, 0.95))
                                           : (FishUI.Theme.darkMode ? Qt.rgba(0.1, 0.1, 0.12, 0.6) : Qt.rgba(1, 1, 1, 0.75))
            border.width: 1
            border.color: FishUI.Theme.glassBorderColor
            visible: viewer.currentIndex >= 0 && viewer.currentIndex < photoModel.count - 1
            opacity: viewportMouseArea.containsMouse || nextMouse.containsMouse ? 1.0 : 0.0

            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }

            FishUI.IconItem {
                anchors.centerIn: parent
                width: 20
                height: 20
                source: "qrc:/images/cp-go-next.svg"
                color: FishUI.Theme.textColor
            }

            MouseArea {
                id: nextMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: viewer.nextRequested()
            }
        }

        // Bottom Filmstrip
        FilmStrip {
            id: filmStrip
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 14
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(parent.width - 40, Math.max(300, photoModel.count * 68 + 24))
            currentIndex: Math.max(0, viewer.currentIndex)
            visible: viewer.filmStripVisible && photoModel.count > 1 && viewer.currentIndex >= 0
            opacity: visible ? 1.0 : 0.0

            Behavior on opacity {
                NumberAnimation { duration: 180 }
            }

            onItemSelected: {
                rootWindow.openViewer(index)
            }
        }
    }

    // Slide-in Metadata Inspector Panel
    InfoPanel {
        id: infoPanel
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        filePath: viewer.currentPath
        visible: viewer.infoPanelVisible
        x: viewer.infoPanelVisible ? parent.width - width : parent.width

        onCloseRequested: {
            viewer.infoPanelVisible = false
        }
    }

    // -----------------------------------------------------------------
    // Context Menu for Viewer Operations
    // -----------------------------------------------------------------
    FishUI.DesktopMenu {
        id: viewerContextMenu

        FishUI.MenuItem {
            readonly property bool isFav: PhotosApp.isFavorite(viewer.currentPath)
            text: isFav ? qsTr("Remove from Favorites") : qsTr("Add to Favorites")
            icon.source: isFav ? "qrc:/images/cp-favorite-filled.svg" : "qrc:/images/cp-favorite.svg"
            onTriggered: PhotosApp.toggleFavorite(viewer.currentPath)
        }

        FishUI.MenuItem {
            text: qsTr("Copy Image")
            icon.source: "qrc:/images/cp-copy.svg"
            onTriggered: PhotosApp.copyImageToClipboard(viewer.currentPath)
        }

        FishUI.MenuSeparator {}

        FishUI.MenuItem {
            text: qsTr("Fit to Screen (0)")
            icon.source: "qrc:/images/cp-fit-screen.svg"
            onTriggered: viewer.fitToScreen()
        }

        FishUI.MenuItem {
            text: qsTr("Actual Size 1:1 (1)")
            icon.source: "qrc:/images/cp-actual-size.svg"
            onTriggered: viewer.setZoom(1.0)
        }

        FishUI.MenuItem {
            text: qsTr("Zoom In (+)")
            icon.source: "qrc:/images/cp-zoom-in.svg"
            onTriggered: viewer.zoomIn()
        }

        FishUI.MenuItem {
            text: qsTr("Zoom Out (-)")
            icon.source: "qrc:/images/cp-zoom-out.svg"
            onTriggered: viewer.zoomOut()
        }

        FishUI.MenuSeparator {}

        FishUI.MenuItem {
            text: qsTr("Rotate Clockwise (R)")
            icon.source: "qrc:/images/cp-rotate-right.svg"
            onTriggered: viewer.rotateClockwise()
        }

        FishUI.MenuItem {
            text: qsTr("Rotate Counter-Clockwise (L)")
            icon.source: "qrc:/images/cp-rotate-left.svg"
            onTriggered: viewer.rotateCounterClockwise()
        }

        FishUI.MenuItem {
            text: qsTr("Flip Horizontal (H)")
            icon.source: "qrc:/images/cp-flip-horizontal.svg"
            onTriggered: viewer.toggleFlip()
        }

        FishUI.MenuSeparator {}

        FishUI.MenuItem {
            text: viewer.filmStripVisible ? qsTr("Hide Filmstrip (S)") : qsTr("Show Filmstrip (S)")
            icon.source: "qrc:/images/cp-filmstrip.svg"
            onTriggered: viewer.filmStripVisible = !viewer.filmStripVisible
        }

        FishUI.MenuItem {
            text: viewer.infoPanelVisible ? qsTr("Hide Inspector (I)") : qsTr("Show Inspector (I)")
            icon.source: "qrc:/images/cp-info.svg"
            onTriggered: viewer.infoPanelVisible = !viewer.infoPanelVisible
        }

        FishUI.MenuItem {
            text: qsTr("Toggle Fullscreen (F11)")
            icon.source: "qrc:/images/cp-fullscreen.svg"
            onTriggered: {
                if (rootWindow.visibility === Window.FullScreen) {
                    rootWindow.showNormal()
                } else {
                    rootWindow.showFullScreen()
                }
            }
        }

        FishUI.MenuSeparator {}

        FishUI.MenuItem {
            text: qsTr("Move to Trash (Delete)")
            icon.source: "qrc:/images/cp-trash.svg"
            onTriggered: rootWindow.confirmDeletePhoto(viewer.currentIndex)
        }
    }

    // -----------------------------------------------------------------
    // Keyboard Shortcuts
    // -----------------------------------------------------------------
    Shortcut {
        sequence: "Left"
        enabled: viewer.visible
        onActivated: viewer.prevRequested()
    }
    Shortcut {
        sequence: "Right"
        enabled: viewer.visible
        onActivated: viewer.nextRequested()
    }
    Shortcut {
        sequence: "Escape"
        enabled: viewer.visible
        onActivated: viewer.backRequested()
    }
    Shortcut {
        sequence: "Backspace"
        enabled: viewer.visible
        onActivated: viewer.backRequested()
    }
    Shortcut {
        sequence: "+"
        enabled: viewer.visible
        onActivated: viewer.zoomIn()
    }
    Shortcut {
        sequence: "="
        enabled: viewer.visible
        onActivated: viewer.zoomIn()
    }
    Shortcut {
        sequence: "-"
        enabled: viewer.visible
        onActivated: viewer.zoomOut()
    }
    Shortcut {
        sequence: "0"
        enabled: viewer.visible
        onActivated: viewer.fitToScreen()
    }
    Shortcut {
        sequence: "1"
        enabled: viewer.visible
        onActivated: viewer.setZoom(1.0)
    }
    Shortcut {
        sequence: "R"
        enabled: viewer.visible
        onActivated: viewer.rotateClockwise()
    }
    Shortcut {
        sequence: "Shift+R"
        enabled: viewer.visible
        onActivated: viewer.rotateCounterClockwise()
    }
    Shortcut {
        sequence: "L"
        enabled: viewer.visible
        onActivated: viewer.rotateCounterClockwise()
    }
    Shortcut {
        sequence: "H"
        enabled: viewer.visible
        onActivated: viewer.toggleFlip()
    }
    Shortcut {
        sequence: "I"
        enabled: viewer.visible
        onActivated: viewer.infoPanelVisible = !viewer.infoPanelVisible
    }
    Shortcut {
        sequence: "S"
        enabled: viewer.visible
        onActivated: viewer.filmStripVisible = !viewer.filmStripVisible
    }
    Shortcut {
        sequence: "F"
        enabled: viewer.visible
        onActivated: {
            if (rootWindow.visibility === Window.FullScreen) {
                rootWindow.showNormal()
            } else {
                rootWindow.showFullScreen()
            }
        }
    }
    Shortcut {
        sequence: "F11"
        enabled: viewer.visible
        onActivated: {
            if (rootWindow.visibility === Window.FullScreen) {
                rootWindow.showNormal()
            } else {
                rootWindow.showFullScreen()
            }
        }
    }
    Shortcut {
        sequence: "Delete"
        enabled: viewer.visible && viewer.currentIndex >= 0
        onActivated: rootWindow.confirmDeletePhoto(viewer.currentIndex)
    }
}
