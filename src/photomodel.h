/*
 * Copyright (C) 2026 CutefishOS Team.
 * Copyright (C) 2026 Project Aevis Contributors.
 *
 * Fast Asynchronous Photo Model for Cutefish Photos Gallery
 */

#ifndef PHOTOMODEL_H
#define PHOTOMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QList>
#include <QUrl>
#include <QDateTime>
#include <QVariantMap>
#include <QFileSystemWatcher>
#include <QAtomicInt>

struct PhotoItem {
    QString name;
    QString path;
    QUrl url;
    QString fileSize;
    qint64 sizeBytes = 0;
    QString modified;
    qint64 modifiedTimestamp = 0;
    QString dimensions;
    int width = 0;
    int height = 0;
    bool isFavorite = false;
    QString mimeType;
};

class PhotoModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentPathChanged)
    Q_PROPERTY(QString currentType READ currentType NOTIFY currentTypeChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(int sortRole READ sortRole WRITE setSortRole NOTIFY sortChanged)
    Q_PROPERTY(bool sortDescending READ sortDescending WRITE setSortDescending NOTIFY sortChanged)

public:
    enum PhotoRoles {
        NameRole = Qt::UserRole + 1,
        PathRole,
        UrlRole,
        FileSizeRole,
        SizeBytesRole,
        ModifiedRole,
        ModifiedTimestampRole,
        DimensionsRole,
        WidthRole,
        HeightRole,
        IsFavoriteRole,
        MimeTypeRole
    };

    enum SortType {
        SortByDate = 0,
        SortByName = 1,
        SortBySize = 2
    };

    explicit PhotoModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_filteredItems.size(); }
    bool loading() const { return m_loading; }
    QString currentPath() const { return m_currentPath; }
    QString currentType() const { return m_currentType; }
    QString filterText() const { return m_filterText; }
    int sortRole() const { return m_sortRole; }
    bool sortDescending() const { return m_sortDescending; }

    Q_INVOKABLE void load(const QString &path, const QString &type = "folder");
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setFilterText(const QString &text);
    Q_INVOKABLE void setSortRole(int role);
    Q_INVOKABLE void setSortDescending(bool descending);
    Q_INVOKABLE QVariantMap get(int index) const;
    Q_INVOKABLE int indexOfPath(const QString &path) const;
    Q_INVOKABLE QString pathOfIndex(int index) const;
    Q_INVOKABLE void toggleFavorite(int index);
    Q_INVOKABLE bool deletePhoto(int index);

signals:
    void countChanged();
    void loadingChanged();
    void currentPathChanged();
    void currentTypeChanged();
    void filterTextChanged();
    void sortChanged();

private:
    void scanPhotosAsync(int scanId);
    void applyFilterAndSort();
    void updateFavorites();

    QList<PhotoItem> m_allItems;
    QList<PhotoItem> m_filteredItems;
    QString m_currentPath;
    QString m_currentType = "all";
    QString m_filterText;
    int m_sortRole = SortByDate;
    bool m_sortDescending = true;
    bool m_loading = false;
    QFileSystemWatcher *m_watcher;
    QAtomicInt m_scanCounter;
};

#endif // PHOTOMODEL_H
