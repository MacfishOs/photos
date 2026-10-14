/*
 * Copyright (C) 2026 CutefishOS Team.
 *
 * Fast Asynchronous Photo Model Implementation
 */

#include "photomodel.h"
#include "photosapp.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QStandardPaths>
#include <QThreadPool>
#include <QLocale>
#include <algorithm>
#include <QDebug>

static const QStringList supportedImageExtensions = {
    "*.jpg", "*.jpeg", "*.png", "*.webp", "*.bmp", "*.gif",
    "*.svg", "*.tiff", "*.tif", "*.ico", "*.avif", "*.heic"
};

static QString formatBytesFast(qint64 bytes)
{
    if (bytes < 1024)
        return QString("%1 B").arg(bytes);
    if (bytes < 1024 * 1024)
        return QString("%1 KB").arg(QString::number(bytes / 1024.0, 'f', 1));
    if (bytes < 1024 * 1024 * 1024)
        return QString("%1 MB").arg(QString::number(bytes / (1024.0 * 1024.0), 'f', 1));
    return QString("%1 GB").arg(QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 1));
}

PhotoModel::PhotoModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_watcher(new QFileSystemWatcher(this))
    , m_scanCounter(0)
{
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &PhotoModel::refresh);

    if (PhotosApp::instance()) {
        connect(PhotosApp::instance(), &PhotosApp::favoritesChanged, this, &PhotoModel::updateFavorites);
    }

    load("", "all");
}

int PhotoModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_filteredItems.size();
}

QVariant PhotoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filteredItems.size())
        return QVariant();

    const PhotoItem &item = m_filteredItems.at(index.row());
    switch (role) {
    case NameRole:
        return item.name;
    case PathRole:
        return item.path;
    case UrlRole:
        return item.url;
    case FileSizeRole:
        return item.fileSize;
    case SizeBytesRole:
        return item.sizeBytes;
    case ModifiedRole:
        return item.modified;
    case ModifiedTimestampRole:
        return item.modifiedTimestamp;
    case DimensionsRole:
        return item.dimensions;
    case WidthRole:
        return item.width;
    case HeightRole:
        return item.height;
    case IsFavoriteRole:
        return item.isFavorite;
    case MimeTypeRole:
        return item.mimeType;
    default:
        break;
    }
    return QVariant();
}

QHash<int, QByteArray> PhotoModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[PathRole] = "path";
    roles[UrlRole] = "url";
    roles[FileSizeRole] = "fileSize";
    roles[SizeBytesRole] = "sizeBytes";
    roles[ModifiedRole] = "modified";
    roles[ModifiedTimestampRole] = "modifiedTimestamp";
    roles[DimensionsRole] = "dimensions";
    roles[WidthRole] = "width";
    roles[HeightRole] = "height";
    roles[IsFavoriteRole] = "isFavorite";
    roles[MimeTypeRole] = "mimeType";
    return roles;
}

QVariantMap PhotoModel::get(int index) const
{
    QVariantMap map;
    if (index < 0 || index >= m_filteredItems.size())
        return map;

    const PhotoItem &item = m_filteredItems.at(index);
    map["name"] = item.name;
    map["path"] = item.path;
    map["url"] = item.url;
    map["fileSize"] = item.fileSize;
    map["sizeBytes"] = item.sizeBytes;
    map["modified"] = item.modified;
    map["modifiedTimestamp"] = item.modifiedTimestamp;
    map["dimensions"] = item.dimensions;
    map["width"] = item.width;
    map["height"] = item.height;
    map["isFavorite"] = item.isFavorite;
    map["mimeType"] = item.mimeType;
    map["index"] = index;
    return map;
}

int PhotoModel::indexOfPath(const QString &path) const
{
    for (int i = 0; i < m_filteredItems.size(); ++i) {
        if (m_filteredItems.at(i).path == path)
            return i;
    }
    return -1;
}

QString PhotoModel::pathOfIndex(int index) const
{
    if (index >= 0 && index < m_filteredItems.size())
        return m_filteredItems.at(index).path;
    return QString();
}

void PhotoModel::load(const QString &path, const QString &type)
{
    if (m_currentPath == path && m_currentType == type) {
        refresh();
        return;
    }

    m_currentPath = path;
    m_currentType = type;
    emit currentPathChanged();
    emit currentTypeChanged();

    // Update watcher
    QStringList watched = m_watcher->directories();
    if (!watched.isEmpty()) {
        m_watcher->removePaths(watched);
    }
    if (!path.isEmpty() && QDir(path).exists()) {
        m_watcher->addPath(path);
    }

    refresh();
}

void PhotoModel::refresh()
{
    const int scanId = m_scanCounter.fetchAndAddRelaxed(1) + 1;
    m_loading = true;
    emit loadingChanged();

    scanPhotosAsync(scanId);
}

void PhotoModel::scanPhotosAsync(int scanId)
{
    const QString type = m_currentType;
    const QString folderPath = m_currentPath;

    // Snapshot favorites for thread safety
    QSet<QString> favSet;
    if (PhotosApp::instance()) {
        const QStringList favs = PhotosApp::instance()->favoritePaths();
        favSet = QSet<QString>(favs.begin(), favs.end());
    }

    QThreadPool::globalInstance()->start([this, scanId, type, folderPath, favSet]() {
        QList<PhotoItem> items;
        QSet<QString> visitedPaths;

        auto processFile = [&](const QFileInfo &fi) {
            const QString absPath = fi.absoluteFilePath();
            if (visitedPaths.contains(absPath))
                return;
            visitedPaths.insert(absPath);

            PhotoItem item;
            item.name = fi.fileName();
            item.path = absPath;
            item.url = QUrl::fromLocalFile(absPath);
            item.sizeBytes = fi.size();
            item.fileSize = formatBytesFast(item.sizeBytes);
            item.modified = QLocale().toString(fi.lastModified(), QLocale::ShortFormat);
            item.modifiedTimestamp = fi.lastModified().toMSecsSinceEpoch();
            item.isFavorite = favSet.contains(absPath);
            item.mimeType = "image/" + fi.suffix().toLower();
            item.dimensions = "";

            items.append(item);
        };

        if (type == "favorites") {
            for (const QString &fPath : favSet) {
                QFileInfo fi(fPath);
                if (fi.exists() && fi.isFile()) {
                    processFile(fi);
                }
            }
        } else if (type == "all") {
            const QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
            const QString wallpapersPath = "/usr/share/wallpapers";

            auto scanDir = [&](const QString &dirPath) {
                if (!dirPath.isEmpty() && QDir(dirPath).exists()) {
                    QDirIterator it(dirPath, supportedImageExtensions,
                                    QDir::Files | QDir::Readable | QDir::NoDotAndDotDot,
                                    QDirIterator::Subdirectories);
                    while (it.hasNext()) {
                        it.next();
                        processFile(it.fileInfo());
                    }
                }
            };

            scanDir(picturesPath);
            scanDir(wallpapersPath);
        } else {
            // Specific folder
            if (!folderPath.isEmpty() && QDir(folderPath).exists()) {
                QDir dir(folderPath);
                QFileInfoList list = dir.entryInfoList(supportedImageExtensions,
                                                      QDir::Files | QDir::Readable | QDir::NoDotAndDotDot,
                                                      QDir::Time);
                for (const QFileInfo &fi : list) {
                    processFile(fi);
                }
            }
        }

        // Post back to GUI thread
        QMetaObject::invokeMethod(this, [this, scanId, items]() {
            if (scanId != m_scanCounter.loadRelaxed()) {
                return; // Discard outdated scan
            }

            m_allItems = items;
            applyFilterAndSort();

            m_loading = false;
            emit loadingChanged();
            emit countChanged();
        }, Qt::QueuedConnection);
    });
}

void PhotoModel::applyFilterAndSort()
{
    beginResetModel();
    m_filteredItems.clear();

    const QString query = m_filterText.trimmed();
    for (const PhotoItem &it : m_allItems) {
        if (query.isEmpty() || it.name.contains(query, Qt::CaseInsensitive)) {
            m_filteredItems.append(it);
        }
    }

    auto comparator = [this](const PhotoItem &a, const PhotoItem &b) {
        if (m_sortRole == SortByName) {
            int comp = QString::compare(a.name, b.name, Qt::CaseInsensitive);
            return m_sortDescending ? comp > 0 : comp < 0;
        } else if (m_sortRole == SortBySize) {
            return m_sortDescending ? a.sizeBytes > b.sizeBytes : a.sizeBytes < b.sizeBytes;
        } else {
            // SortByDate
            return m_sortDescending ? a.modifiedTimestamp > b.modifiedTimestamp
                                    : a.modifiedTimestamp < b.modifiedTimestamp;
        }
    };

    std::sort(m_filteredItems.begin(), m_filteredItems.end(), comparator);
    endResetModel();
}

void PhotoModel::updateFavorites()
{
    PhotosApp *app = PhotosApp::instance();
    if (!app)
        return;

    for (int i = 0; i < m_filteredItems.size(); ++i) {
        bool fav = app->isFavorite(m_filteredItems.at(i).path);
        if (m_filteredItems.at(i).isFavorite != fav) {
            m_filteredItems[i].isFavorite = fav;
            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {IsFavoriteRole});
        }
    }

    for (int i = 0; i < m_allItems.size(); ++i) {
        m_allItems[i].isFavorite = app->isFavorite(m_allItems.at(i).path);
    }

    if (m_currentType == "favorites") {
        refresh();
    }
}

void PhotoModel::setFilterText(const QString &text)
{
    if (m_filterText == text)
        return;

    m_filterText = text;
    emit filterTextChanged();
    applyFilterAndSort();
    emit countChanged();
}

void PhotoModel::setSortRole(int role)
{
    if (m_sortRole == role)
        return;

    m_sortRole = role;
    emit sortChanged();
    applyFilterAndSort();
}

void PhotoModel::setSortDescending(bool descending)
{
    if (m_sortDescending == descending)
        return;

    m_sortDescending = descending;
    emit sortChanged();
    applyFilterAndSort();
}

void PhotoModel::toggleFavorite(int index)
{
    if (index < 0 || index >= m_filteredItems.size())
        return;

    const QString path = m_filteredItems.at(index).path;
    PhotosApp *app = PhotosApp::instance();
    if (app) {
        app->toggleFavorite(path);
    }
}

bool PhotoModel::deletePhoto(int index)
{
    if (index < 0 || index >= m_filteredItems.size())
        return false;

    const QString path = m_filteredItems.at(index).path;
    PhotosApp *app = PhotosApp::instance();
    if (!app)
        return false;

    bool ok = app->deletePhoto(path);
    if (ok) {
        // Remove locally from items
        beginResetModel();
        m_filteredItems.removeAt(index);
        for (int i = 0; i < m_allItems.size(); ++i) {
            if (m_allItems.at(i).path == path) {
                m_allItems.removeAt(i);
                break;
            }
        }
        endResetModel();
        emit countChanged();
    }
    return ok;
}
