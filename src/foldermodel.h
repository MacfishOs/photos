/*
 * Copyright (C) 2026 CutefishOS Team.
 * Copyright (C) 2026 Project Aevis Contributors.
 *
 * Folder Model for Cutefish Photos Sidebar
 */

#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QList>
#include <QFileSystemWatcher>

struct FolderItem {
    QString name;
    QString path;
    QString icon;
    QString section;
    QString typeId;    // "all", "favorites", "folder"
    int count = 0;
    bool isCustom = false;
};

class FolderModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum FolderRoles {
        NameRole = Qt::UserRole + 1,
        PathRole,
        IconRole,
        SectionRole,
        TypeIdRole,
        CountRole,
        IsCustomRole
    };

    explicit FolderModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void addCustomFolder(const QString &folderPath);
    Q_INVOKABLE void removeCustomFolder(const QString &folderPath);
    Q_INVOKABLE int indexOfPath(const QString &path) const;
    Q_INVOKABLE int indexOfType(const QString &typeId) const;

signals:
    void folderCountUpdated();

private:
    void populateStandardFolders();
    void scanSubfolders();
    void loadCustomFolders();
    void saveCustomFolders();
    int countPhotosInDir(const QString &dirPath, bool recursive = false) const;
    void updateCounts();

    QList<FolderItem> m_items;
    QStringList m_customFolders;
    QFileSystemWatcher *m_watcher;
};

#endif // FOLDERMODEL_H
