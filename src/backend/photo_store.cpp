// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "backend/photo_store.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <thread>

namespace fs = std::filesystem;

namespace cutefish::photos {

PhotoStore& PhotoStore::instance()
{
    static PhotoStore store;
    return store;
}

PhotoStore::PhotoStore()
{
    loadFavorites();
    scanPhotos();
}

PhotoStore::~PhotoStore()
{
    saveFavorites();
}

void PhotoStore::setCategory(NavCategory category)
{
    m_category = category;
    m_customDirPath.clear();
    m_activeFolderPath.clear();
    applyFilterAndSort();
}

void PhotoStore::setSegment(LibraryViewSegment seg)
{
    if (m_segment == seg) return;
    m_segment = seg;
    applyFilterAndSort();
}

void PhotoStore::loadDirectory(const std::string& dirPath, const std::string& initialFilePath)
{
    m_category = NavCategory::CustomFolder;
    m_customDirPath = dirPath;
    m_activeFolderPath = dirPath;
    scanPhotos();
}

void PhotoStore::setSearchQuery(const std::string& query)
{
    m_searchQuery = query;
    applyFilterAndSort();
}

bool PhotoStore::isSupportedImage(const std::string& path) const
{
    std::string ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".webp" ||
            ext == ".bmp" || ext == ".svg" || ext == ".gif" || ext == ".tiff" ||
            ext == ".heic" || ext == ".avif" || ext == ".mp4" || ext == ".mov" ||
            ext == ".mkv" || ext == ".webm");
}

void PhotoStore::filterByFolderPath(const std::string& folderPath)
{
    m_category = NavCategory::CustomFolder;
    m_activeFolderPath = folderPath;
    applyFilterAndSort();
}

static bool isIgnoredDirectoryName(const std::string& name)
{
    if (name.empty()) return false;
    if (name[0] == '.') return true;
    return (name == "node_modules" || name == "build" || name == "target" ||
            name == "__MACOSX" || name == "gradle" || name == "bin" ||
            name == "obj" || name == "dist" || name == "out");
}

void PhotoStore::scanPhotos()
{
    m_allPhotos.clear();
    m_folders.clear();

    const char* home = std::getenv("HOME");
    std::string homeDir = home ? home : "/home/jatin";

    std::vector<std::string> baseDirs = {
        homeDir + "/Pictures",
        homeDir + "/Downloads",
        homeDir + "/Desktop",
        homeDir + "/Documents"
    };

    std::vector<std::string> searchDirs;

    if (!m_customDirPath.empty()) {
        searchDirs.push_back(m_customDirPath);
    } else {
        for (const auto& base : baseDirs) {
            std::error_code ec;
            if (fs::exists(base, ec) && fs::is_directory(base, ec)) {
                searchDirs.push_back(base);
            }
        }
    }

    auto nowTime = std::chrono::system_clock::now();
    std::map<std::string, int> folderCounts;

    for (const auto& dir : searchDirs) {
        std::error_code ec;
        if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
            continue;
        }

        try {
            for (auto it = fs::recursive_directory_iterator(dir, fs::directory_options::skip_permission_denied, ec);
                 it != fs::recursive_directory_iterator(); ++it) {
                if (ec) break;
                if (it.depth() > 6) {
                    it.pop();
                    if (it == fs::recursive_directory_iterator()) break;
                    continue;
                }
                const auto& entry = *it;
                std::string filename = entry.path().filename().string();

                if (entry.is_directory(ec)) {
                    if (isIgnoredDirectoryName(filename)) {
                        it.disable_recursion_pending();
                    }
                    continue;
                }

                if (!entry.is_regular_file(ec)) {
                    continue;
                }

                std::string filePath = entry.path().string();
                if (!isSupportedImage(filePath)) {
                    continue;
                }

                std::string parentDir = entry.path().parent_path().string();
                folderCounts[parentDir]++;

                if (m_allPhotos.size() < 3000) {
                    PhotoItem item;
                    item.path = filePath;
                    item.title = filename;
                    item.isFavorite = (m_favoritePaths.find(filePath) != m_favoritePaths.end());
                    item.isTrash = (m_trashPaths.find(filePath) != m_trashPaths.end());

                    std::string ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    if (ext == ".mp4" || ext == ".mov" || ext == ".mkv" || ext == ".webm") {
                        item.isVideo = true;
                    }

                    auto ftime = entry.last_write_time(ec);
                    if (!ec) {
                        auto s_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());

                        std::time_t ctime = std::chrono::system_clock::to_time_t(s_time);
                        std::stringstream ss;
                        ss << std::put_time(std::localtime(&ctime), "%b %d, %Y");
                        item.date = ss.str();

                        auto ageDays = std::chrono::duration_cast<std::chrono::hours>(nowTime - s_time).count() / 24;
                        if (ageDays <= 30) {
                            item.isRecentlySaved = true;
                        }
                    } else {
                        item.date = "Oct 16, 2024";
                    }

                    auto fsize = entry.file_size(ec);
                    item.sizeBytes = ec ? 0 : static_cast<long>(fsize);

                    m_allPhotos.push_back(item);
                }
            }
        } catch (...) {
        }
    }

    if (m_allPhotos.empty()) {
        std::vector<std::string> sysDirs = {
            "/usr/share/backgrounds",
            "/usr/share/wallpapers"
        };
        for (const auto& dir : sysDirs) {
            if (!fs::exists(dir) || !fs::is_directory(dir)) continue;
            try {
                for (auto it = fs::recursive_directory_iterator(dir, fs::directory_options::skip_permission_denied);
                     it != fs::recursive_directory_iterator(); ++it) {
                    if (it.depth() > 2) {
                        it.pop();
                        if (it == fs::recursive_directory_iterator()) break;
                        continue;
                    }
                    const auto& entry = *it;
                    if (!entry.is_regular_file()) continue;
                    std::string filePath = entry.path().string();
                    if (!isSupportedImage(filePath)) continue;

                    PhotoItem item;
                    item.path = filePath;
                    item.title = entry.path().filename().string();
                    item.isFavorite = (m_favoritePaths.find(filePath) != m_favoritePaths.end());
                    item.isTrash = (m_trashPaths.find(filePath) != m_trashPaths.end());
                    item.date = "System Wallpapers";
                    try { item.sizeBytes = static_cast<long>(entry.file_size()); } catch (...) {}
                    m_allPhotos.push_back(item);

                    std::string parentDir = entry.path().parent_path().string();
                    folderCounts[parentDir]++;

                    if (m_allPhotos.size() >= 100) break;
                }
            } catch (...) {}
            if (m_allPhotos.size() >= 100) break;
        }
    }

    // Build m_folders list
    for (const auto& [dirPath, count] : folderCounts) {
        FolderItem folder;
        folder.path = dirPath;
        folder.parentPath = fs::path(dirPath).parent_path().string();
        folder.name = fs::path(dirPath).filename().string();
        folder.photoCount = count;

        int indent = 0;
        for (const auto& base : baseDirs) {
            if (dirPath.rfind(base, 0) == 0 && dirPath.length() > base.length()) {
                std::string sub = dirPath.substr(base.length() + 1);
                indent = 1 + static_cast<int>(std::count(sub.begin(), sub.end(), '/'));
                break;
            }
        }
        folder.indentLevel = indent;
        m_folders.push_back(folder);
    }

    // Sort folders hierarchically by path
    std::sort(m_folders.begin(), m_folders.end(), [](const FolderItem& a, const FolderItem& b) {
        return a.path < b.path;
    });

    // Sort photos by path / modification time descending
    std::sort(m_allPhotos.begin(), m_allPhotos.end(), [](const PhotoItem& a, const PhotoItem& b) {
        return a.path > b.path;
    });

    generateSampleAlbums();
    applyFilterAndSort();
}

void PhotoStore::generateSampleAlbums()
{
    m_albums.clear();

    std::vector<std::string> titles = {
        "Top Moments",
        "Close Friends",
        "Coastal Days",
        "Scenic Views",
        "Family Time",
        "Food Creations"
    };

    std::vector<std::string> ids = {
        "top_moments",
        "close_friends",
        "coastal_days",
        "scenic_views",
        "family_time",
        "food_creations"
    };

    for (size_t i = 0; i < titles.size(); ++i) {
        AlbumItem album;
        album.id = ids[i];
        album.title = titles[i];

        int count = 0;
        for (size_t p = 0; p < m_allPhotos.size(); ++p) {
            if (p % titles.size() == i) {
                m_allPhotos[p].albumId = album.id;
                if (album.coverPhotoPath.empty()) {
                    album.coverPhotoPath = m_allPhotos[p].path;
                }
                count++;
            }
        }

        album.photoCount = count;
        if (album.coverPhotoPath.empty() && !m_allPhotos.empty()) {
            album.coverPhotoPath = m_allPhotos[0].path;
        }

        m_albums.push_back(album);
    }
}

void PhotoStore::applyFilterAndSort()
{
    m_filteredPhotos.clear();

    const char* home = std::getenv("HOME");
    std::string homeDir = home ? home : "/home/jatin";

    for (const auto& item : m_allPhotos) {
        if (item.isTrash) continue;

        // Category filter
        if (m_category == NavCategory::CustomFolder && !m_activeFolderPath.empty()) {
            if (fs::path(item.path).parent_path().string() != m_activeFolderPath &&
                item.path.rfind(m_activeFolderPath, 0) != 0) {
                continue;
            }
        } else if (m_category == NavCategory::Favorites && !item.isFavorite) {
            continue;
        } else if (m_category == NavCategory::RecentlySaved && !item.isRecentlySaved) {
            continue;
        } else if (m_category == NavCategory::FolderPictures) {
            std::string picPath = homeDir + "/Pictures";
            if (item.path.rfind(picPath, 0) != 0) continue;
        } else if (m_category == NavCategory::FolderDownloads) {
            std::string dlPath = homeDir + "/Downloads";
            if (item.path.rfind(dlPath, 0) != 0) continue;
        } else if (m_category == NavCategory::FolderDesktop) {
            std::string dtPath = homeDir + "/Desktop";
            if (item.path.rfind(dtPath, 0) != 0) continue;
        } else if (m_category == NavCategory::FolderDocuments &&
                   item.path.find("/Documents") == std::string::npos &&
                   item.path.find("/documents") == std::string::npos) {
            continue;
        }

        // Search query filter
        if (!m_searchQuery.empty()) {
            std::string titleLower = item.title;
            std::string queryLower = m_searchQuery;
            std::transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::tolower);
            std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(), ::tolower);
            if (titleLower.find(queryLower) == std::string::npos) {
                continue;
            }
        }

        m_filteredPhotos.push_back(item);
    }

    photosChanged.emit();
}

size_t PhotoStore::countForCategory(NavCategory category) const
{
    size_t count = 0;
    for (const auto& item : m_allPhotos) {
        if (item.isTrash) continue;

        if (category == NavCategory::Library) {
            count++;
        } else if (category == NavCategory::Favorites && item.isFavorite) {
            count++;
        } else if (category == NavCategory::RecentlySaved && item.isRecentlySaved) {
            count++;
        } else if (category == NavCategory::FolderPictures && item.path.find("/Pictures") != std::string::npos) {
            count++;
        } else if (category == NavCategory::FolderDownloads && item.path.find("/Downloads") != std::string::npos) {
            count++;
        } else if (category == NavCategory::FolderDesktop && item.path.find("/Desktop") != std::string::npos) {
            count++;
        } else if (category == NavCategory::FolderDocuments && item.path.find("/Documents") != std::string::npos) {
            count++;
        }
    }
    return count;
}

void PhotoStore::toggleFavorite(const std::string& path)
{
    if (m_favoritePaths.find(path) != m_favoritePaths.end()) {
        m_favoritePaths.erase(path);
    } else {
        m_favoritePaths.insert(path);
    }

    for (auto& item : m_allPhotos) {
        if (item.path == path) {
            item.isFavorite = (m_favoritePaths.find(path) != m_favoritePaths.end());
        }
    }

    saveFavorites();
    applyFilterAndSort();
}

void PhotoStore::moveToTrash(const std::string& path)
{
    m_trashPaths.insert(path);
    for (auto& item : m_allPhotos) {
        if (item.path == path) {
            item.isTrash = true;
        }
    }
    applyFilterAndSort();
}

void PhotoStore::restoreFromTrash(const std::string& path)
{
    m_trashPaths.erase(path);
    for (auto& item : m_allPhotos) {
        if (item.path == path) {
            item.isTrash = false;
        }
    }
    applyFilterAndSort();
}

void PhotoStore::deletePermanently(const std::string& path)
{
    m_trashPaths.erase(path);
    m_favoritePaths.erase(path);
    try {
        fs::remove(path);
    } catch (...) {
    }

    m_allPhotos.erase(std::remove_if(m_allPhotos.begin(), m_allPhotos.end(),
                                     [&path](const PhotoItem& item) { return item.path == path; }),
                      m_allPhotos.end());

    applyFilterAndSort();
}

bool PhotoStore::setAsWallpaper(const std::string& path)
{
    if (path.empty() || !fs::exists(path)) return false;

    // Cutefish Desktop Wallpaper / XDG setter command
    std::string cmd = "cutefish-set-wallpaper \"" + path + "\" 2>/dev/null &";
    std::system(cmd.c_str());
    return true;
}

void PhotoStore::loadFavorites()
{
    const char* home = std::getenv("HOME");
    std::string favFile = home ? (std::string(home) + "/.config/cutefish-photos-favorites.txt") : "/tmp/cutefish-photos-favorites.txt";

    std::ifstream infile(favFile);
    std::string line;
    while (std::getline(infile, line)) {
        if (!line.empty()) {
            m_favoritePaths.insert(line);
        }
    }
}

void PhotoStore::saveFavorites()
{
    const char* home = std::getenv("HOME");
    std::string favFile = home ? (std::string(home) + "/.config/cutefish-photos-favorites.txt") : "/tmp/cutefish-photos-favorites.txt";

    std::ofstream outfile(favFile);
    for (const auto& path : m_favoritePaths) {
        outfile << path << "\n";
    }
}

} // namespace cutefish::photos
