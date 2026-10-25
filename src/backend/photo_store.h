// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "base/signal.h"
#include <string>
#include <vector>
#include <set>

namespace cutefish::photos {

struct PhotoItem
{
    std::string path;
    std::string title;
    std::string date;
    std::string albumId;
    long sizeBytes = 0;
    int width = 0;
    int height = 0;
    bool isFavorite = false;
    bool isTrash = false;
    bool isVideo = false;
    bool isRecentlySaved = false;

    std::string formattedSize() const
    {
        if (sizeBytes < 1024) {
            return std::to_string(sizeBytes) + " B";
        } else if (sizeBytes < 1024 * 1024) {
            return std::to_string(sizeBytes / 1024) + " KB";
        } else {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.1f MB", static_cast<double>(sizeBytes) / (1024.0 * 1024.0));
            return std::string(buf);
        }
    }

    std::string formattedDimensions() const
    {
        if (width > 0 && height > 0) {
            return std::to_string(width) + " × " + std::to_string(height);
        }
        return "Unknown dimensions";
    }

    std::string formattedDate() const
    {
        return date.empty() ? "Oct 16, 2024" : date;
    }
};

struct AlbumItem
{
    std::string id;
    std::string title;
    std::string coverPhotoPath;
    int photoCount = 0;
};

struct FolderItem
{
    std::string path;
    std::string name;
    std::string parentPath;
    int indentLevel = 0;
    int photoCount = 0;
};

enum class NavCategory
{
    Library,
    Favorites,
    RecentlySaved,

    FolderPictures,
    FolderDownloads,
    FolderDesktop,
    FolderDocuments,

    CustomFolder
};

enum class LibraryViewSegment
{
    Years,
    Months,
    AllPhotos
};

class PhotoStore
{
public:
    static PhotoStore& instance();

    PhotoStore();
    ~PhotoStore();

    void setCategory(NavCategory category);
    NavCategory category() const { return m_category; }

    void setSegment(LibraryViewSegment seg);
    LibraryViewSegment segment() const { return m_segment; }

    void loadDirectory(const std::string& dirPath, const std::string& initialFilePath = {});
    void setSearchQuery(const std::string& query);
    std::string searchQuery() const { return m_searchQuery; }

    const std::vector<PhotoItem>& photos() const { return m_filteredPhotos; }
    const std::vector<PhotoItem>& allLoadedPhotos() const { return m_allPhotos; }
    const std::vector<AlbumItem>& albums() const { return m_albums; }
    const std::vector<FolderItem>& folders() const { return m_folders; }

    void filterByFolderPath(const std::string& folderPath);
    std::string activeFolderPath() const { return m_activeFolderPath; }

    size_t countForCategory(NavCategory category) const;

    bool isFavorite(const std::string& path) const { return m_favoritePaths.find(path) != m_favoritePaths.end(); }
    void toggleFavorite(const std::string& path);
    void moveToTrash(const std::string& path);
    void deletePermanently(const std::string& path);
    void restoreFromTrash(const std::string& path);

    static bool setAsWallpaper(const std::string& path);

    base::Signal<> photosChanged;

private:
    void scanPhotos();
    void applyFilterAndSort();
    void generateSampleAlbums();
    void loadFavorites();
    void saveFavorites();
    bool isSupportedImage(const std::string& path) const;

    NavCategory m_category = NavCategory::Library;
    LibraryViewSegment m_segment = LibraryViewSegment::AllPhotos;
    std::string m_customDirPath;
    std::string m_searchQuery;

    std::vector<PhotoItem> m_allPhotos;
    std::vector<PhotoItem> m_filteredPhotos;
    std::vector<AlbumItem> m_albums;
    std::vector<FolderItem> m_folders;
    std::string m_activeFolderPath;
    std::set<std::string> m_favoritePaths;
    std::set<std::string> m_trashPaths;
};

} // namespace cutefish::photos
