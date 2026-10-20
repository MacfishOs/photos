/*
 * Copyright (C) 2021 CutefishOS Team.
 * Copyright (C) 2026 Project Aevis Contributors.
 *
 * Project Aevis Photos Application Backend Implementation
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

static QString photosConfigFile()
{
    QString aevisConfig = QDir::homePath() + "/.config/aevis/aevis-photos.conf";
    if (QFile::exists(aevisConfig)) {
        return aevisConfig;
    }
    QString legacyConfig = QDir::homePath() + "/.config/cutefishos/cutefish-photos.conf";
    if (QFile::exists(legacyConfig)) {
        return legacyConfig;
    }
    return aevisConfig;
}

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
    QSettings settings(photosConfigFile(), QSettings::IniFormat);
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
    QString configPath = photosConfigFile();
    QDir().mkpath(QFileInfo(configPath).dir().absolutePath());
    QSettings settings(configPath, QSettings::IniFormat);
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

    // 1. Try launching Aevis or Cutefish file manager directly
    if (QProcess::startDetached("aevis-filemanager", { "--select", filePath }) ||
        QProcess::startDetached("cutefish-filemanager", { "--select", filePath })) {
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

    // 1. DBus call to ThemeManager (Aevis / Cutefish Services)
    struct DBusTarget {
        const char *service;
        const char *path;
        const char *interface;
    };

    static const DBusTarget targets[] = {
        { "org.aevis.Services", "/org/aevis/Services/Appearance", "org.aevis.Services.Appearance" },
        { "com.cutefish.Services", "/com/cutefish/Services/Appearance", "com.cutefish.Services.Appearance" },
        { "org.aevis.Services", "/org/aevis/Services/Appearance", "com.cutefish.Services.Appearance" },
        { "com.cutefish.Services", "/com/cutefish/Services/Appearance", "org.aevis.Services.Appearance" },
    };

    for (const auto &target : targets) {
        QDBusInterface iface(QString::fromLatin1(target.service),
                             QString::fromLatin1(target.path),
                             QString::fromLatin1(target.interface),
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            iface.call(QStringLiteral("setWallpaper"), filePath);
            iface.call(QStringLiteral("setBackgroundType"), 0);
            success = true;
            break;
        }
    }

    // 2. Direct config save to ~/.config/aevis/theme.conf and ~/.config/cutefishos/theme.conf
    const QString aevisDir = QDir::homePath() + QStringLiteral("/.config/aevis");
    const QString cutefishDir = QDir::homePath() + QStringLiteral("/.config/cutefishos");
    QDir().mkpath(aevisDir);
    QDir().mkpath(cutefishDir);

    QSettings aevisSettings(aevisDir + QStringLiteral("/theme.conf"), QSettings::IniFormat);
    aevisSettings.setValue(QStringLiteral("Wallpaper"), filePath);
    aevisSettings.setValue(QStringLiteral("BackgroundType"), 0);
    aevisSettings.sync();

    QSettings cutefishSettings(cutefishDir + QStringLiteral("/theme.conf"), QSettings::IniFormat);
    cutefishSettings.setValue(QStringLiteral("Wallpaper"), filePath);
    cutefishSettings.setValue(QStringLiteral("BackgroundType"), 0);
    cutefishSettings.sync();

    // 3. Fallback for GNOME/Cinnamon/MATE environments
    QProcess::execute("gsettings", { "set", "org.gnome.desktop.background", "picture-uri", QUrl::fromLocalFile(filePath).toString() });
    QProcess::execute("gsettings", { "set", "org.gnome.desktop.background", "picture-uri-dark", QUrl::fromLocalFile(filePath).toString() });

    emit actionFeedback(tr("Desktop wallpaper set"), true);
    return true;
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
