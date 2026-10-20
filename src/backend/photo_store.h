// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "base/signal.h"
#include <string>
#include <vector>

namespace cutefish::photos {

struct PhotoItem
{
    std::string path;
    std::string title;
    std::string date;
    long sizeBytes = 0;
    bool isFavorite = false;
};

enum class NavCategory
{
    Photos,
    Favorites,
    Desktop,
    Documents,
    Downloads,
    Screenshots,
    Trash
};

class PhotoStore
{
public:
    static PhotoStore& instance();

    PhotoStore();
    ~PhotoStore();

    void loadPhotos(NavCategory category = NavCategory::Photos);
    const std::vector<PhotoItem>& photos() const { return m_photos; }

    void toggleFavorite(const std::string& path);
    void deletePhoto(const std::string& path);

    base::Signal<> photosChanged;

private:
    std::vector<PhotoItem> m_photos;
    NavCategory m_currentCategory = NavCategory::Photos;
};

} // namespace cutefish::photos
