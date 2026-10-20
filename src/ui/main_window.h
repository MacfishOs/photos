// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "backend/photo_store.h"
#include "ui/view.h"
#include "ui/window.h"
#include "ui/header_bar.h"
#include "ui/sidebar_view.h"
#include "ui/views/gallery_view.h"
#include "ui/views/image_viewer.h"

namespace cutefish::photos {

class PhotosRootView : public ui::View
{
public:
    PhotosRootView();
    ~PhotosRootView() override;

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;

private:
    SidebarView* m_sidebar = nullptr;
    HeaderBar* m_headerBar = nullptr;
    GalleryView* m_galleryView = nullptr;
    ImageViewer* m_imageViewer = nullptr;

    void showGallery();
    void showViewer(const PhotoItem& photo);
};

class MainWindow : public ui::Window
{
public:
    MainWindow();
    ~MainWindow() override;

private:
    PhotosRootView* m_rootView = nullptr;
};

} // namespace cutefish::photos
