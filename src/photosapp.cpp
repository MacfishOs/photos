/*
 * Copyright (C) 2026 CutefishOS Team.
 *
 * Cutefish Photos Application Backend Implementation
 */

#include "photosapp.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QImage>
#include <QImageReader>
#include <QMimeDatabase>
#include <QProcess>
#include <QDesktopServices>
#include <QSettings>
#include <QStandardPaths>
#include <QFileDialog>
#include <QDateTime>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDebug>
#include <numeric>

PhotosApp *PhotosApp::s_instance = nullptr;

PhotosApp::PhotosApp(QObject *parent)
    : QObject(parent)
{
    s_instance = this;
    loadFavorites();
}

PhotosApp *PhotosApp::instance()
{
    return s_instance;
}

void PhotosApp::loadFavorites()
{
    QSettings settings(QDir::homePath() + "/.config/cutefishos/cutefish-photos.conf", QSettings::IniFormat);
    settings.beginGroup("Favorites");
    QStringList list = settings.value("photos").toStringList();
    settings.endGroup();

    m_favorites.clear();
    for (const QString &path : list) {
        if (QFile::exists(path)) {
            m_favorites.insert(path);
        }
    }
}

void PhotosApp::saveFavorites()
{
    QSettings settings(QDir::homePath() + "/.config/cutefishos/cutefish-photos.conf", QSettings::IniFormat);
    settings.beginGroup("Favorites");
    QStringList list;
    for (const QString &path : m_favorites) {
        list.append(path);
    }
    settings.setValue("photos", list);
    settings.endGroup();
    settings.sync();
}

QStringList PhotosApp::favoritePaths() const
{
    QStringList list;
    for (const QString &path : m_favorites) {
        list.append(path);
    }
    return list;
}

bool PhotosApp::isFavorite(const QString &filePath) const
{
    return m_favorites.contains(filePath);
}

void PhotosApp::toggleFavorite(const QString &filePath)
{
    setFavorite(filePath, !isFavorite(filePath));
}

void PhotosApp::setFavorite(const QString &filePath, bool fav)
{
    if (filePath.isEmpty())
        return;

    if (fav) {
        m_favorites.insert(filePath);
        emit actionFeedback(tr("Added to Favorites"), true);
    } else {
        m_favorites.remove(filePath);
        emit actionFeedback(tr("Removed from Favorites"), true);
    }
    saveFavorites();
    emit favoritesChanged();
}

bool PhotosApp::copyImageToClipboard(const QString &filePath)
{
    if (filePath.isEmpty() || !QFile::exists(filePath)) {
        emit actionFeedback(tr("Image file not found"), false);
        return false;
    }

    QImage image(filePath);
    if (image.isNull()) {
        emit actionFeedback(tr("Could not load image to copy"), false);
        return false;
    }

    QMimeData *mimeData = new QMimeData();
    mimeData->setImageData(image);
    mimeData->setUrls({ QUrl::fromLocalFile(filePath) });
    mimeData->setText(filePath);
    QGuiApplication::clipboard()->setMimeData(mimeData);

    emit actionFeedback(tr("Image copied to clipboard"), true);
    return true;
}

bool PhotosApp::copyPathToClipboard(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    QGuiApplication::clipboard()->setText(filePath);
    emit actionFeedback(tr("File path copied to clipboard"), true);
    return true;
}

bool PhotosApp::showInFileManager(const QString &filePath)
{
    if (filePath.isEmpty() || !QFile::exists(filePath))
        return false;

    // 1. Try launching Cutefish file manager directly
    if (QProcess::startDetached("cutefish-filemanager", { "--select", filePath })) {
        emit actionFeedback(tr("Opened in File Manager"), true);
        return true;
    }

    // 2. Try standard Freedesktop DBus file manager interface
    QDBusInterface fileManager("org.freedesktop.FileManager1",
                               "/org/freedesktop/FileManager1",
                               "org.freedesktop.FileManager1",
                               QDBusConnection::sessionBus());
    if (fileManager.isValid()) {
        QStringList uriList = { QUrl::fromLocalFile(filePath).toString() };
        fileManager.call("ShowItems", uriList, QString());
        emit actionFeedback(tr("Opened in File Manager"), true);
        return true;
    }

    // 3. Fallback: Open directory via system handler
    QFileInfo fi(filePath);
    bool ok = QDesktopServices::openUrl(QUrl::fromLocalFile(fi.dir().absolutePath()));
    emit actionFeedback(tr("Opened containing folder"), ok);
    return ok;
}

bool PhotosApp::openWithDefaultApp(const QString &filePath)
{
    if (filePath.isEmpty() || !QFile::exists(filePath))
        return false;

    bool ok = QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    emit actionFeedback(tr("Opened with default app"), ok);
    return ok;
}

bool PhotosApp::setWallpaper(const QString &filePath)
{
    if (filePath.isEmpty() || !QFile::exists(filePath)) {
        emit actionFeedback(tr("Image does not exist"), false);
        return false;
    }

    bool success = false;

    // 1. Cutefish Appearance DBus Service
    QDBusInterface cutefishAppearance("org.cutefish.Appearance",
                                      "/org/cutefish/Appearance",
                                      "org.cutefish.Appearance",
                                      QDBusConnection::sessionBus());
    if (cutefishAppearance.isValid()) {
        cutefishAppearance.call("setWallpaper", filePath);
        success = true;
    }

    // 2. Cutefish settings file (~/.config/cutefishos/cutefish.conf)
    QString configDir = QDir::homePath() + "/.config/cutefishos";
    QDir().mkpath(configDir);
    QSettings cfSettings(configDir + "/cutefish.conf", QSettings::IniFormat);
    cfSettings.beginGroup("Appearance");
    cfSettings.setValue("wallpaper", filePath);
    cfSettings.endGroup();
    cfSettings.sync();
    success = true;

    // 3. Fallback for GNOME/Cinnamon/MATE environments if run outside Cutefish shell
    QProcess::execute("gsettings", { "set", "org.gnome.desktop.background", "picture-uri", QUrl::fromLocalFile(filePath).toString() });
    QProcess::execute("gsettings", { "set", "org.gnome.desktop.background", "picture-uri-dark", QUrl::fromLocalFile(filePath).toString() });

    emit actionFeedback(tr("Desktop wallpaper set"), success);
    return success;
}

bool PhotosApp::deletePhoto(const QString &filePath)
{
    if (filePath.isEmpty() || !QFile::exists(filePath))
        return false;

    QFile file(filePath);
    bool ok = file.moveToTrash();
    if (!ok) {
        ok = file.remove();
    }

    if (ok) {
        if (m_favorites.contains(filePath)) {
            m_favorites.remove(filePath);
            saveFavorites();
            emit favoritesChanged();
        }
        emit actionFeedback(tr("Photo moved to Trash"), true);
    } else {
        emit actionFeedback(tr("Failed to delete photo"), false);
    }

    return ok;
}

QString PhotosApp::pickFolder(const QString &title)
{
    QString initialDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString selected = QFileDialog::getExistingDirectory(nullptr,
        title.isEmpty() ? tr("Select Photos Folder") : title,
        initialDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    return selected;
}

QString PhotosApp::formatFileSize(qint64 bytes) const
{
    if (bytes < 1024)
        return QString("%1 B").arg(bytes);
    if (bytes < 1024 * 1024)
        return QString("%1 KB").arg(QString::number((double)bytes / 1024.0, 'f', 1));
    if (bytes < 1024 * 1024 * 1024)
        return QString("%1 MB").arg(QString::number((double)bytes / (1024.0 * 1024.0), 'f', 1));
    return QString("%1 GB").arg(QString::number((double)bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2));
}

QVariantMap PhotosApp::getImageMetadata(const QString &filePath) const
{
    QVariantMap map;
    QFileInfo fi(filePath);
    if (!fi.exists())
        return map;

    map["fileName"] = fi.fileName();
    map["filePath"] = fi.absoluteFilePath();
    map["fileSize"] = formatFileSize(fi.size());
    map["sizeBytes"] = fi.size();
    map["modified"] = QLocale().toString(fi.lastModified(), QLocale::ShortFormat);
    map["created"] = fi.birthTime().isValid() ? QLocale().toString(fi.birthTime(), QLocale::ShortFormat) : QLocale().toString(fi.lastModified(), QLocale::ShortFormat);
    map["suffix"] = fi.suffix().toUpper();

    QImageReader reader(filePath);
    reader.setAutoTransform(true);
    QSize size = reader.size();
    if (size.isValid()) {
        map["width"] = size.width();
        map["height"] = size.height();
        map["dimensions"] = QString("%1 × %2").arg(size.width()).arg(size.height());
        double mp = (double)(size.width() * size.height()) / 1000000.0;
        map["megapixels"] = QString("%1 MP").arg(QString::number(mp, 'f', 1));

        int gcdVal = std::gcd(size.width(), size.height());
        if (gcdVal > 0 && (size.width() / gcdVal) < 40 && (size.height() / gcdVal) < 40) {
            map["aspectRatio"] = QString("%1:%2").arg(size.width() / gcdVal).arg(size.height() / gcdVal);
        } else {
            double ratio = (double)size.width() / (double)size.height();
            map["aspectRatio"] = QString("%1:1").arg(QString::number(ratio, 'f', 2));
        }
    } else {
        map["width"] = 0;
        map["height"] = 0;
        map["dimensions"] = tr("Unknown");
        map["megapixels"] = "";
        map["aspectRatio"] = "";
    }

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(fi);
    map["mimeType"] = mime.name();

    return map;
}

QUrl PhotosApp::localFileUrl(const QString &path) const
{
    return QUrl::fromLocalFile(path);
}

QString PhotosApp::fileName(const QString &path) const
{
    return QFileInfo(path).fileName();
}

QString PhotosApp::parentDirectory(const QString &path) const
{
    return QFileInfo(path).dir().absolutePath();
}
