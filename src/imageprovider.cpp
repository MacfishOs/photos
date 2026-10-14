/*
 * Copyright (C) 2026 CutefishOS Team.
 *
 * Fast Thumbnail Image Provider Implementation with Two-tier Caching
 */

#include "imageprovider.h"

#include <QImageReader>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QUrlQuery>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QMutexLocker>
#include <QDebug>

ThumbnailProvider::ThumbnailProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    // 64 MB fast in-memory thumbnail cache
    m_cache.setMaxCost(64 * 1024 * 1024);

    // Persistent disk thumbnail cache
    QString cacheBase = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (cacheBase.isEmpty()) {
        cacheBase = QDir::homePath() + "/.cache/cutefish-photos";
    }
    m_diskCacheDir = cacheBase + "/thumbnails";
    QDir().mkpath(m_diskCacheDir);
}

ThumbnailProvider::~ThumbnailProvider()
{
}

QImage ThumbnailProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString filePath = id;
    int reqWidth = requestedSize.width();
    int reqHeight = requestedSize.height();

    // Check if query parameters are included in the id string
    int queryIdx = id.indexOf('?');
    if (queryIdx != -1) {
        filePath = id.left(queryIdx);
        QString queryStr = id.mid(queryIdx + 1);
        QUrlQuery query(queryStr);
        if (query.hasQueryItem("w")) {
            reqWidth = query.queryItemValue("w").toInt();
        }
        if (query.hasQueryItem("h")) {
            reqHeight = query.queryItemValue("h").toInt();
        }
    }

    // Decode URL formatting if present
    filePath = QUrl::fromPercentEncoding(filePath.toUtf8());

    if (filePath.isEmpty()) {
        return QImage();
    }

    QFileInfo fi(filePath);
    if (!fi.exists() || !fi.isFile()) {
        return QImage();
    }

    if (reqWidth <= 0 && reqHeight <= 0) {
        reqWidth = 256;
        reqHeight = 256;
    } else if (reqWidth <= 0) {
        reqWidth = reqHeight;
    } else if (reqHeight <= 0) {
        reqHeight = reqWidth;
    }

    // Step 1: Check In-Memory RAM Cache
    const QString memCacheKey = QString("%1@%2x%3").arg(filePath).arg(reqWidth).arg(reqHeight);
    {
        QMutexLocker locker(&m_mutex);
        if (m_cache.contains(memCacheKey)) {
            QImage *cached = m_cache.object(memCacheKey);
            if (cached && !cached->isNull()) {
                if (size)
                    *size = cached->size();
                return *cached;
            }
        }
    }

    // Step 2: Check Persistent Fast Disk Cache
    // Hash includes canonical path, size, mtime, target width and height
    const qint64 mtime = fi.lastModified().toMSecsSinceEpoch();
    const qint64 fsize = fi.size();
    const QString hashInput = QString("%1:%2:%3:%4x%5")
                                  .arg(fi.canonicalFilePath())
                                  .arg(fsize)
                                  .arg(mtime)
                                  .arg(reqWidth)
                                  .arg(reqHeight);

    const QString diskCacheFilename = QString(QCryptographicHash::hash(hashInput.toUtf8(), QCryptographicHash::Md5).toHex()) + ".jpg";
    const QString diskCacheFilePath = m_diskCacheDir + "/" + diskCacheFilename;

    if (QFileInfo::exists(diskCacheFilePath)) {
        QImage diskCached(diskCacheFilePath);
        if (!diskCached.isNull()) {
            if (size)
                *size = diskCached.size();

            QMutexLocker locker(&m_mutex);
            m_cache.insert(memCacheKey, new QImage(diskCached), diskCached.sizeInBytes());
            return diskCached;
        }
    }

    // Step 3: Decode & Downscale Image
    QImageReader reader(filePath);
    reader.setAutoTransform(true);
    QSize origSize = reader.size();

    if (!origSize.isValid()) {
        return QImage();
    }

    QSize targetSize(reqWidth, reqHeight);
    origSize.scale(targetSize, Qt::KeepAspectRatio);
    reader.setScaledSize(origSize);
    reader.setQuality(80);

    QImage img = reader.read();
    if (!img.isNull()) {
        if (size)
            *size = img.size();

        // Save to persistent disk cache asynchronously/synchronously
        img.save(diskCacheFilePath, "JPG", 82);

        QMutexLocker locker(&m_mutex);
        m_cache.insert(memCacheKey, new QImage(img), img.sizeInBytes());
        return img;
    }

    return QImage();
}
