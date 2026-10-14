/*
 * Copyright (C) 2026 CutefishOS Team.
 *
 * Folder Model Implementation
 */

#include "foldermodel.h"
#include "photosapp.h"

#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>
#include <QSettings>
#include <QFileInfo>
#include <QSet>
#include <QDebug>

static const QStringList supportedImageExtensions = {
    "*.jpg", "*.jpeg", "*.png", "*.webp", "*.bmp", "*.gif",
    "*.svg", "*.tiff", "*.tif", "*.ico", "*.avif", "*.heic"
};

FolderModel::FolderModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_watcher(new QFileSystemWatcher(this))
{
    loadCustomFolders();
    refresh();

    if (PhotosApp::instance()) {
        connect(PhotosApp::instance(), &PhotosApp::favoritesChanged, this, &FolderModel::updateCounts);
    }
}

int FolderModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant FolderModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    const FolderItem &item = m_items.at(index.row());
    switch (role) {
    case NameRole:
        return item.name;
    case PathRole:
        return item.path;
    case IconRole:
        return item.icon;
    case SectionRole:
        return item.section;
    case TypeIdRole:
        return item.typeId;
    case CountRole:
        return item.count;
    case IsCustomRole:
        return item.isCustom;
    default:
        break;
    }
    return QVariant();
}

QHash<int, QByteArray> FolderModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[PathRole] = "path";
    roles[IconRole] = "icon";
    roles[SectionRole] = "section";
    roles[TypeIdRole] = "typeId";
    roles[CountRole] = "count";
    roles[IsCustomRole] = "isCustom";
    return roles;
}

void FolderModel::loadCustomFolders()
{
    QSettings settings(QDir::homePath() + "/.config/cutefishos/cutefish-photos.conf", QSettings::IniFormat);
    settings.beginGroup("CustomFolders");
    m_customFolders = settings.value("paths").toStringList();
    settings.endGroup();
}

void FolderModel::saveCustomFolders()
{
    QSettings settings(QDir::homePath() + "/.config/cutefishos/cutefish-photos.conf", QSettings::IniFormat);
    settings.beginGroup("CustomFolders");
    settings.setValue("paths", m_customFolders);
    settings.endGroup();
    settings.sync();
}

void FolderModel::addCustomFolder(const QString &folderPath)
{
    if (folderPath.isEmpty() || !QDir(folderPath).exists())
        return;

    QString clean = QDir::cleanPath(folderPath);
    if (!m_customFolders.contains(clean)) {
        m_customFolders.append(clean);
        saveCustomFolders();
        refresh();
    }
}

void FolderModel::removeCustomFolder(const QString &folderPath)
{
    QString clean = QDir::cleanPath(folderPath);
    if (m_customFolders.removeAll(clean) > 0) {
        saveCustomFolders();
        refresh();
    }
}

int FolderModel::countPhotosInDir(const QString &dirPath, bool recursive) const
{
    if (dirPath.isEmpty() || !QDir(dirPath).exists())
        return 0;

    int count = 0;
    QDir::Filters filters = QDir::Files | QDir::Readable | QDir::NoDotAndDotDot;
    if (recursive) {
        QDirIterator it(dirPath, supportedImageExtensions, filters, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            count++;
        }
    } else {
        QDir dir(dirPath);
        count = dir.entryList(supportedImageExtensions, filters).size();
    }
    return count;
}

void FolderModel::populateStandardFolders()
{
    QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString homePath = QDir::homePath();
    QString screenshotsPath = picturesPath + "/Screenshots";
    QString wallpapersPath = "/usr/share/wallpapers";

    // 1. Library Section
    m_items.append({ tr("All Photos"), "", "qrc:/images/sidebar-photos.svg", tr("Library"), "all", 0, false });
    m_items.append({ tr("Favorites"), "", "qrc:/images/sidebar-favorites.svg", tr("Library"), "favorites", 0, false });

    if (QDir(screenshotsPath).exists()) {
        m_items.append({ tr("Screenshots"), screenshotsPath, "qrc:/images/sidebar-screenshots.svg", tr("Library"), "folder", 0, false });
    }

    if (QDir(wallpapersPath).exists()) {
        m_items.append({ tr("Wallpapers"), wallpapersPath, "qrc:/images/sidebar-wallpapers.svg", tr("Library"), "folder", 0, false });
    }

    // 2. Places Section
    if (QDir(picturesPath).exists()) {
        m_items.append({ tr("Pictures"), picturesPath, "qrc:/images/sidebar-pictures.svg", tr("Places"), "folder", 0, false });
    }
    if (QDir(downloadsPath).exists()) {
        m_items.append({ tr("Downloads"), downloadsPath, "qrc:/images/sidebar-downloads.svg", tr("Places"), "folder", 0, false });
    }
    if (QDir(desktopPath).exists()) {
        m_items.append({ tr("Desktop"), desktopPath, "qrc:/images/sidebar-desktop.svg", tr("Places"), "folder", 0, false });
    }
    if (QDir(documentsPath).exists()) {
        m_items.append({ tr("Documents"), documentsPath, "qrc:/images/sidebar-documents.svg", tr("Places"), "folder", 0, false });
    }
    if (QDir(homePath).exists()) {
        m_items.append({ tr("Home"), homePath, "qrc:/images/sidebar-home.svg", tr("Places"), "folder", 0, false });
    }
}

void FolderModel::scanSubfolders()
{
    QSet<QString> addedPaths;
    for (const FolderItem &it : m_items) {
        if (!it.path.isEmpty())
            addedPaths.insert(QDir::cleanPath(it.path));
    }

    // Scan subfolders under Pictures
    QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (QDir(picturesPath).exists()) {
        QDir picDir(picturesPath);
        QStringList subDirs = picDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name);
        for (const QString &sub : subDirs) {
            QString fullSub = picDir.absoluteFilePath(sub);
            if (!addedPaths.contains(fullSub)) {
                addedPaths.insert(fullSub);
                m_items.append({ sub, fullSub, "qrc:/images/sidebar-folder.svg", tr("All Folders"), "folder", 0, false });
            }
        }
    }

    // Scan subfolders under Downloads that contain photos
    QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (QDir(downloadsPath).exists()) {
        QDir dlDir(downloadsPath);
        QStringList dlSubDirs = dlDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name);
        for (const QString &sub : dlSubDirs) {
            QString fullSub = dlDir.absoluteFilePath(sub);
            if (!addedPaths.contains(fullSub) && countPhotosInDir(fullSub, false) > 0) {
                addedPaths.insert(fullSub);
                m_items.append({ sub, fullSub, "qrc:/images/sidebar-folder.svg", tr("All Folders"), "folder", 0, false });
            }
        }
    }

    // Add custom folders
    for (const QString &custom : m_customFolders) {
        if (QDir(custom).exists() && !addedPaths.contains(custom)) {
            addedPaths.insert(custom);
            QFileInfo fi(custom);
            m_items.append({ fi.fileName(), custom, "qrc:/images/sidebar-folder.svg", tr("All Folders"), "folder", 0, true });
        }
    }
}

void FolderModel::updateCounts()
{
    int allCount = 0;
    QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    allCount += countPhotosInDir(picturesPath, true);

    QString wallpapersPath = "/usr/share/wallpapers";
    if (QDir(wallpapersPath).exists()) {
        allCount += countPhotosInDir(wallpapersPath, true);
    }

    int favCount = 0;
    if (PhotosApp::instance()) {
        favCount = PhotosApp::instance()->favoritePaths().size();
    }

    for (int i = 0; i < m_items.size(); ++i) {
        FolderItem &it = m_items[i];
        if (it.typeId == "all") {
            it.count = allCount;
        } else if (it.typeId == "favorites") {
            it.count = favCount;
        } else if (!it.path.isEmpty()) {
            it.count = countPhotosInDir(it.path, false);
        }
    }

    emit dataChanged(index(0), index(m_items.size() - 1), { CountRole });
    emit folderCountUpdated();
}

void FolderModel::refresh()
{
    beginResetModel();
    m_items.clear();
    populateStandardFolders();
    scanSubfolders();
    endResetModel();

    updateCounts();
}

int FolderModel::indexOfPath(const QString &path) const
{
    QString clean = QDir::cleanPath(path);
    for (int i = 0; i < m_items.size(); ++i) {
        if (!m_items.at(i).path.isEmpty() && QDir::cleanPath(m_items.at(i).path) == clean) {
            return i;
        }
    }
    return -1;
}

int FolderModel::indexOfType(const QString &typeId) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).typeId == typeId) {
            return i;
        }
    }
    return -1;
}
