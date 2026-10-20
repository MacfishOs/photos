// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "backend/photo_store.h"
#include "base/json.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace cutefish::photos {

PhotoStore& PhotoStore::instance()
{
    static PhotoStore s_instance;
    return s_instance;
}

PhotoStore::PhotoStore()
{
    loadPhotos(NavCategory::Photos);
}

PhotoStore::~PhotoStore()
{
}

void PhotoStore::loadPhotos(NavCategory category)
{
    m_currentCategory = category;
    m_photos.clear();

    const char* home = std::getenv("HOME");
    fs::path homeDir(home ? home : ".");

    std::vector<fs::path> searchDirs;
    switch (category) {
    case NavCategory::Photos:
        searchDirs.push_back(homeDir / "Pictures");
        searchDirs.push_back(homeDir / "Photos");
        break;
    case NavCategory::Desktop:
        searchDirs.push_back(homeDir / "Desktop");
        break;
    case NavCategory::Documents:
        searchDirs.push_back(homeDir / "Documents");
        break;
    case NavCategory::Downloads:
        searchDirs.push_back(homeDir / "Downloads");
        break;
    case NavCategory::Screenshots:
        searchDirs.push_back(homeDir / "Pictures" / "Screenshots");
        searchDirs.push_back(homeDir / "Desktop");
        break;
    case NavCategory::Favorites:
    case NavCategory::Trash:
        searchDirs.push_back(homeDir / "Pictures");
        break;
    }

    std::vector<std::string> exts = {".jpg", ".jpeg", ".png", ".webp", ".gif", ".JPG", ".JPEG", ".PNG"};

    for (const auto& dir : searchDirs) {
        if (!fs::exists(dir) || !fs::is_directory(dir))
            continue;

        std::error_code ec;
        for (const auto& entry : fs::recursive_directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (std::find(exts.begin(), exts.end(), ext) != exts.end()) {
                    PhotoItem item;
                    item.path = entry.path().string();
                    item.title = entry.path().stem().string();

                    auto ftime = fs::last_write_time(entry.path(), ec);
                    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
                    char buf[64];
                    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&cftime));
                    item.date = buf;

                    item.sizeBytes = entry.file_size(ec);

                    if (category == NavCategory::Favorites) {
                        item.isFavorite = true;
                    }

                    m_photos.push_back(item);
                }
            }
        }
    }

    if (m_photos.empty()) {
        m_photos.push_back({
            "https://images.unsplash.com/photo-1506744038136-46273834b3fb?w=1000",
            "Yosemite Valley", "2026-03-01", 2048000, true
        });
        m_photos.push_back({
            "https://images.unsplash.com/photo-1470071459604-3b5ec3a7fe05?w=1000",
            "Misty Pine Forest", "2026-03-02", 3145728, false
        });
        m_photos.push_back({
            "https://images.unsplash.com/photo-1426604966848-d7adac902bff?w=1000",
            "Alpine Mountain Lake", "2026-03-03", 1835008, true
        });
        m_photos.push_back({
            "https://images.unsplash.com/photo-1511497584788-876761142212?w=1000",
            "Northern Lights", "2026-03-04", 4194304, false
        });
        m_photos.push_back({
            "https://images.unsplash.com/photo-1469474968028-56623f02e42e?w=1000",
            "Sunset Horizon", "2026-03-05", 2516582, true
        });
    }

    photosChanged.emit();
}

void PhotoStore::toggleFavorite(const std::string& path)
{
    auto it = std::find_if(m_photos.begin(), m_photos.end(), [&](const PhotoItem& p) { return p.path == path; });
    if (it != m_photos.end()) {
        it->isFavorite = !it->isFavorite;
        photosChanged.emit();
    }
}

void PhotoStore::deletePhoto(const std::string& path)
{
    auto it = std::find_if(m_photos.begin(), m_photos.end(), [&](const PhotoItem& p) { return p.path == path; });
    if (it != m_photos.end()) {
        std::error_code ec;
        fs::remove(path, ec);
        m_photos.erase(it);
        photosChanged.emit();
    }
}

} // namespace cutefish::photos
