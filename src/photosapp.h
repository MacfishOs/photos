/*
 * Copyright (C) 2026 CutefishOS Team.
 *
 * Cutefish Photos Application Backend
 */

#ifndef PHOTOSAPP_H
#define PHOTOSAPP_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>
#include <QSet>

class PhotosApp : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList favoritePaths READ favoritePaths NOTIFY favoritesChanged)

public:
    explicit PhotosApp(QObject *parent = nullptr);
    static PhotosApp *instance();

    QStringList favoritePaths() const;

    Q_INVOKABLE bool isFavorite(const QString &filePath) const;
    Q_INVOKABLE void toggleFavorite(const QString &filePath);
    Q_INVOKABLE void setFavorite(const QString &filePath, bool fav);

    Q_INVOKABLE bool copyImageToClipboard(const QString &filePath);
    Q_INVOKABLE bool copyPathToClipboard(const QString &filePath);
    Q_INVOKABLE bool showInFileManager(const QString &filePath);
    Q_INVOKABLE bool openWithDefaultApp(const QString &filePath);
    Q_INVOKABLE bool setWallpaper(const QString &filePath);
    Q_INVOKABLE bool deletePhoto(const QString &filePath);

    Q_INVOKABLE QString pickFolder(const QString &title = QString());
    Q_INVOKABLE QString formatFileSize(qint64 bytes) const;
    Q_INVOKABLE QVariantMap getImageMetadata(const QString &filePath) const;
    Q_INVOKABLE QUrl localFileUrl(const QString &path) const;
    Q_INVOKABLE QString fileName(const QString &path) const;
    Q_INVOKABLE QString parentDirectory(const QString &path) const;

signals:
    void favoritesChanged();
    void actionFeedback(const QString &message, bool success);

private:
    void loadFavorites();
    void saveFavorites();

    static PhotosApp *s_instance;
    QSet<QString> m_favorites;
};

#endif // PHOTOSAPP_H
