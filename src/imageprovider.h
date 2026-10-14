/*
 * Copyright (C) 2026 CutefishOS Team.
 *
 * Fast Asynchronous / Cached Thumbnail Image Provider
 */

#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QCache>
#include <QMutex>
#include <QString>

class ThumbnailProvider : public QQuickImageProvider
{
public:
    ThumbnailProvider();
    ~ThumbnailProvider() override;

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    QString m_diskCacheDir;
    QCache<QString, QImage> m_cache;
    QMutex m_mutex;
};

#endif // IMAGEPROVIDER_H
