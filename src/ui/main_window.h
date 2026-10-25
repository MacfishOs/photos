// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "backend/photo_store.h"
#include "ui/header_bar.h"
#include "ui/info_panel.h"
#include "ui/sidebar_view.h"
#include "ui/slideshow_view.h"
#include "ui/view.h"
#include "ui/views/gallery_view.h"
#include "ui/views/image_viewer.h"
#include "ui/window.h"

namespace cutefish::photos {

enum class AppViewMode
{
    Gallery,
    Viewer,
    Slideshow
};

class PhotosRootView : public ui::View
{
public:
    PhotosRootView();
    ~PhotosRootView() override = default;

    void openPhoto(const PhotoItem& photo);
    void openDirectory(const std::string& dirPath, const std::string& initialFilePath = {});
    void showToastMessage(const std::string& message);

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;

private:
    void showGallery();
    void showViewer(const PhotoItem& photo);
    void toggleInfoPanel();
    void toggleTheme();
    void startSlideshow();
    void updateHeaderTitle();

    AppViewMode m_mode = AppViewMode::Gallery;
    bool m_showInfoPanel = false;

    HeaderBar* m_headerBar = nullptr;
    SidebarView* m_sidebar = nullptr;
    GalleryView* m_galleryView = nullptr;
    ImageViewer* m_imageViewer = nullptr;
    InfoPanel* m_infoPanel = nullptr;
    SlideshowView* m_slideshowView = nullptr;

    base::Subscription m_storeSub;
    std::vector<base::Subscription> m_subs;
};

class MainWindow : public ui::Window
{
public:
    MainWindow();
    ~MainWindow() override = default;

    void openPhoto(const PhotoItem& photo);
    void openDirectory(const std::string& dirPath, const std::string& initialFilePath = {});
    void updateBackground();

private:
    PhotosRootView* m_rootView = nullptr;
};

} // namespace cutefish::photos
