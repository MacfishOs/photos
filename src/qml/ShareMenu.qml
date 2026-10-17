/*
 * Copyright (C) 2026 CutefishOS Team.
 * Copyright (C) 2026 Project Aevis Contributors.
 *
 * Share Menu for Cutefish Photos
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import FishUI 1.0 as FishUI
import Cutefish.Photos 1.0

FishUI.DesktopMenu {
    id: root

    property string filePath: ""

    FishUI.MenuItem {
        text: qsTr("Copy Image")
        icon.source: "qrc:/images/cp-copy.svg"
        onTriggered: {
            PhotosApp.copyImageToClipboard(root.filePath)
        }
    }

    FishUI.MenuItem {
        text: qsTr("Copy File Path")
        icon.source: "qrc:/images/cp-copy.svg"
        onTriggered: {
            PhotosApp.copyPathToClipboard(root.filePath)
        }
    }

    FishUI.MenuSeparator {}

    FishUI.MenuItem {
        text: qsTr("Show in File Manager")
        icon.source: "qrc:/images/cp-folder-open.svg"
        onTriggered: {
            PhotosApp.showInFileManager(root.filePath)
        }
    }

    FishUI.MenuItem {
        text: qsTr("Set as Desktop Wallpaper")
        icon.source: "qrc:/images/cp-wallpaper.svg"
        onTriggered: {
            PhotosApp.setWallpaper(root.filePath)
        }
    }

    FishUI.MenuSeparator {}

    FishUI.MenuItem {
        text: qsTr("Open with Default Application")
        icon.source: "qrc:/images/cp-folder-open.svg"
        onTriggered: {
            PhotosApp.openWithDefaultApp(root.filePath)
        }
    }
}
