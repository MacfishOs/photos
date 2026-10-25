// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/main_window.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/toast.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace cutefish::photos {

// ============================================================================
// PhotosRootView
// ============================================================================

PhotosRootView::PhotosRootView()
{
    m_headerBar = addChild<HeaderBar>();
    m_sidebar = addChild<SidebarView>();
    m_galleryView = addChild<GalleryView>();
    m_imageViewer = addChild<ImageViewer>();
    m_infoPanel = addChild<InfoPanel>();
    m_slideshowView = addChild<SlideshowView>();

    // Initial visibility state
    m_galleryView->setVisible(true);
    m_imageViewer->setVisible(false);
    m_infoPanel->setVisible(false);
    m_slideshowView->setVisible(false);

    // Wire Header Bar signals
    m_subs.push_back(m_headerBar->backClicked.connect([this]() {
        if (m_mode == AppViewMode::Viewer || m_mode == AppViewMode::Slideshow) {
            showGallery();
        }
    }));

    m_subs.push_back(m_headerBar->themeToggled.connect([this]() {
        toggleTheme();
    }));

    m_subs.push_back(m_headerBar->shareClicked.connect([this]() {
        showToastMessage("1 Photo Selected • Share Sheet Opened");
    }));

    m_subs.push_back(m_headerBar->favoriteClicked.connect([this]() {
        auto photo = m_galleryView->selectedPhoto();
        if (photo) {
            PhotoStore::instance().toggleFavorite(photo->path);
            showToastMessage(photo->isFavorite ? "Added to Favorites" : "Removed from Favorites");
        }
    }));

    m_subs.push_back(m_headerBar->slideshowClicked.connect([this]() {
        startSlideshow();
    }));

    m_subs.push_back(m_headerBar->infoPanelToggled.connect([this]() {
        toggleInfoPanel();
    }));

    m_subs.push_back(m_headerBar->searchChanged.connect([](const std::string& query) {
        PhotoStore::instance().setSearchQuery(query);
    }));

    // Wire Gallery View signals
    m_subs.push_back(m_galleryView->photoClicked.connect([this](const PhotoItem& item) {
        std::cout << "[DEBUG] m_galleryView->photoClicked for: " << item.path << std::endl;
        openPhoto(item);
    }));

    m_subs.push_back(m_galleryView->photoDoubleClicked.connect([this](const PhotoItem& item) {
        std::cout << "[DEBUG] m_galleryView->photoDoubleClicked for: " << item.path << std::endl;
        openPhoto(item);
    }));

    // Wire Sidebar signals
    m_subs.push_back(m_sidebar->categorySelected.connect([this](NavCategory cat) {
        if (m_mode != AppViewMode::Gallery) {
            showGallery();
        }
        (void)cat;
    }));

    // Wire Viewer signals
    m_subs.push_back(m_imageViewer->prevClicked.connect([this]() {
        auto photos = PhotoStore::instance().photos();
        if (photos.empty()) return;
        int idx = m_imageViewer->currentIndex() - 1;
        if (idx < 0) idx = static_cast<int>(photos.size()) - 1;
        showViewer(photos[idx]);
    }));

    m_subs.push_back(m_imageViewer->nextClicked.connect([this]() {
        auto photos = PhotoStore::instance().photos();
        if (photos.empty()) return;
        int idx = (m_imageViewer->currentIndex() + 1) % photos.size();
        showViewer(photos[idx]);
    }));

    m_subs.push_back(m_imageViewer->closeRequested.connect([this]() {
        showGallery();
    }));

    m_subs.push_back(m_imageViewer->shareClicked.connect([this]() {
        showToastMessage("Share Sheet Opened");
    }));

    m_subs.push_back(m_imageViewer->wallpaperSet.connect([this]() {
        if (!m_imageViewer->currentPhoto().path.empty()) {
            PhotoStore::setAsWallpaper(m_imageViewer->currentPhoto().path);
        }
        showToastMessage("Desktop Wallpaper Applied");
    }));

    m_subs.push_back(m_imageViewer->infoPanelToggled.connect([this]() {
        toggleInfoPanel();
    }));

    m_subs.push_back(m_imageViewer->currentPhotoChanged.connect([this](const PhotoItem& item) {
        if (m_showInfoPanel && m_infoPanel) {
            m_infoPanel->setPhoto(&item);
        }
    }));

    // Wire Slideshow signals
    m_subs.push_back(m_slideshowView->exitClicked.connect([this]() {
        showGallery();
    }));

    m_storeSub = PhotoStore::instance().photosChanged.connect([this]() {
        updateHeaderTitle();
    });

    updateHeaderTitle();
}

void PhotosRootView::updateHeaderTitle()
{
    NavCategory cat = PhotoStore::instance().category();
    size_t count = PhotoStore::instance().photos().size();

    if (cat == NavCategory::CustomFolder) {
        std::string folderPath = PhotoStore::instance().activeFolderPath();
        std::string folderName = fs::path(folderPath).filename().string();
        if (folderName.empty()) folderName = "Folder";
        m_headerBar->setTitle(folderName, std::to_string(count) + " Photos");
    } else if (cat == NavCategory::Favorites) {
        m_headerBar->setTitle("Favorites", std::to_string(count) + " Photos");
    } else if (cat == NavCategory::RecentlySaved) {
        m_headerBar->setTitle("Recently Saved", std::to_string(count) + " Photos");
    } else if (cat == NavCategory::FolderPictures) {
        m_headerBar->setTitle("Pictures", std::to_string(count) + " Photos");
    } else if (cat == NavCategory::FolderDownloads) {
        m_headerBar->setTitle("Downloads", std::to_string(count) + " Photos");
    } else if (cat == NavCategory::FolderDesktop) {
        m_headerBar->setTitle("Desktop", std::to_string(count) + " Photos");
    } else {
        m_headerBar->setTitle("Library", std::to_string(count) + " Photos");
    }
}

void PhotosRootView::showToastMessage(const std::string& message)
{
    if (window()) {
        ui::Toast toast(window());
        toast.show(message);
    }
}

void PhotosRootView::openPhoto(const PhotoItem& photo)
{
    std::cout << "[DEBUG] openPhoto called for: " << photo.path << std::endl;
    showViewer(photo);
}

void PhotosRootView::openDirectory(const std::string& dirPath, const std::string& initialFilePath)
{
    std::cout << "[DEBUG] openDirectory called: dirPath=" << dirPath << " initialFile=" << initialFilePath << std::endl;
    PhotoStore::instance().loadDirectory(dirPath, initialFilePath);

    if (!initialFilePath.empty()) {
        auto photos = PhotoStore::instance().allLoadedPhotos();
        for (const auto& item : photos) {
            if (item.path == initialFilePath) {
                openPhoto(item);
                return;
            }
        }

        // Direct fallback item creation for command line image targets
        PhotoItem item;
        item.path = initialFilePath;
        item.title = fs::path(initialFilePath).filename().string();
        openPhoto(item);
        return;
    }
    showGallery();
}

void PhotosRootView::showGallery()
{
    std::cout << "[DEBUG] showGallery called" << std::endl;
    m_mode = AppViewMode::Gallery;
    m_headerBar->setVisible(true);
    m_headerBar->setBackButtonVisible(false);
    m_sidebar->setVisible(true);
    m_galleryView->setVisible(true);
    m_imageViewer->setVisible(false);
    m_slideshowView->setVisible(false);
    m_slideshowView->stop();

    raiseChild(m_sidebar);
    raiseChild(m_headerBar);

    updateHeaderTitle();
    requestLayout();
    requestPaint();
}

void PhotosRootView::showViewer(const PhotoItem& photo)
{
    std::cout << "[DEBUG] showViewer called for: " << photo.path << std::endl;
    m_mode = AppViewMode::Viewer;
    m_headerBar->setVisible(false);
    m_sidebar->setVisible(false);
    m_galleryView->setVisible(false);
    m_imageViewer->setVisible(true);
    m_slideshowView->setVisible(false);

    // Reset geometry so setGeometry in layout() always executes layout
    m_imageViewer->setGeometry({0, 0, 0, 0});
    raiseChild(m_imageViewer);

    if (m_showInfoPanel && m_infoPanel) {
        m_infoPanel->setPhoto(&photo);
        raiseChild(m_infoPanel);
    }

    auto photos = PhotoStore::instance().photos();
    int idx = 0;
    for (size_t i = 0; i < photos.size(); ++i) {
        if (photos[i].path == photo.path) {
            idx = static_cast<int>(i);
            break;
        }
    }

    m_imageViewer->setPhoto(photo, idx, static_cast<int>(photos.size()));
    m_imageViewer->setFocus();

    requestLayout();
    requestPaint();
}

void PhotosRootView::toggleInfoPanel()
{
    m_showInfoPanel = !m_showInfoPanel;
    m_infoPanel->setVisible(m_showInfoPanel);
    if (m_showInfoPanel && m_infoPanel) {
        raiseChild(m_infoPanel);
    }
    requestLayout();
    requestPaint();
}

void PhotosRootView::toggleTheme()
{
    bool isDark = ui::Theme::current().darkMode;
    if (isDark) {
        ui::Theme::setCurrent(ui::Theme::light());
    } else {
        ui::Theme::setCurrent(ui::Theme::dark());
    }
    if (auto win = dynamic_cast<MainWindow*>(window())) {
        win->updateBackground();
    }
    showToastMessage(isDark ? "Light Mode Enabled" : "Dark Mode Enabled");
}

void PhotosRootView::startSlideshow()
{
    auto photos = PhotoStore::instance().photos();
    if (photos.empty()) return;

    m_mode = AppViewMode::Slideshow;
    m_headerBar->setVisible(false);
    m_sidebar->setVisible(false);
    m_galleryView->setVisible(false);
    m_imageViewer->setVisible(false);
    m_infoPanel->setVisible(false);
    m_slideshowView->setVisible(true);

    raiseChild(m_slideshowView);
    m_slideshowView->start(photos, 0);
    requestLayout();
    requestPaint();
}

void PhotosRootView::layout()
{
    double w = localRect().width;
    double h = localRect().height;

    if (m_mode == AppViewMode::Slideshow) {
        m_slideshowView->setGeometry({0, 0, w, h});
        m_slideshowView->setVisible(true);
        m_headerBar->setGeometry({0, 0, 0, 0});
        m_headerBar->setVisible(false);
        m_sidebar->setGeometry({0, 0, 0, 0});
        m_sidebar->setVisible(false);
        m_galleryView->setGeometry({0, 0, 0, 0});
        m_galleryView->setVisible(false);
        m_imageViewer->setGeometry({0, 0, 0, 0});
        m_imageViewer->setVisible(false);
        m_infoPanel->setGeometry({0, 0, 0, 0});
        m_infoPanel->setVisible(false);
        return;
    }

    m_slideshowView->setGeometry({0, 0, 0, 0});
    m_slideshowView->setVisible(false);

    double headerH = 52.0;

    if (m_mode == AppViewMode::Gallery) {
        double sidebarW = 200.0;
        m_sidebar->setGeometry({0, 0, sidebarW, h});
        m_sidebar->setVisible(true);

        double infoW = m_showInfoPanel ? 260.0 : 0.0;
        double availableW = w - sidebarW - infoW;

        m_headerBar->setGeometry({sidebarW, 0, w - sidebarW, headerH});
        m_headerBar->setVisible(true);

        m_galleryView->setGeometry({sidebarW, headerH, availableW, h - headerH});
        m_galleryView->setVisible(true);

        if (m_showInfoPanel) {
            m_infoPanel->setGeometry({w - infoW, headerH, infoW, h - headerH});
            m_infoPanel->setVisible(true);
        } else {
            m_infoPanel->setGeometry({0, 0, 0, 0});
            m_infoPanel->setVisible(false);
        }

        m_imageViewer->setGeometry({0, 0, 0, 0});
        m_imageViewer->setVisible(false);
    } else if (m_mode == AppViewMode::Viewer) {
        m_headerBar->setGeometry({0, 0, 0, 0});
        m_headerBar->setVisible(false);
        m_sidebar->setGeometry({0, 0, 0, 0});
        m_sidebar->setVisible(false);
        m_galleryView->setGeometry({0, 0, 0, 0});
        m_galleryView->setVisible(false);

        double infoW = m_showInfoPanel ? 260.0 : 0.0;
        double availableW = w - infoW;

        m_imageViewer->setGeometry({0, 0, availableW, h});
        m_imageViewer->setVisible(true);

        if (m_showInfoPanel) {
            m_infoPanel->setGeometry({w - infoW, 0, infoW, h});
            m_infoPanel->setVisible(true);
        } else {
            m_infoPanel->setGeometry({0, 0, 0, 0});
            m_infoPanel->setVisible(false);
        }
    }
}

void PhotosRootView::paint(ui::Painter& painter)
{
    bool dark = ui::Theme::current().darkMode;

    if (m_mode == AppViewMode::Viewer || m_mode == AppViewMode::Slideshow) {
        ui::Color contentBg = dark ? ui::Color::fromArgb(0xFF0A0A0C) : ui::Color::fromArgb(0xFFEBEBEF);
        painter.fillRect(localRect(), contentBg);
    } else if (m_mode == AppViewMode::Gallery) {
        double sidebarW = 200.0;
        ui::RectF contentArea{sidebarW, 0, localRect().width - sidebarW, localRect().height};
        ui::Color contentBg = dark ? ui::Color::fromArgb(0xFF18181C) : ui::Color::fromArgb(0xFFFFFFFF);
        painter.fillRect(contentArea, contentBg);
    }
}

// ============================================================================
// MainWindow
// ============================================================================

MainWindow::MainWindow()
{
    setTitle("Photos");
    resize({1100, 720});

    updateBackground();

    auto root = std::make_unique<PhotosRootView>();
    m_rootView = root.get();
    setContentView(std::move(root));
}

void MainWindow::updateBackground()
{
    bool dark = ui::Theme::current().darkMode;
    ui::Color winBg = dark ? ui::Color::fromArgb(0x8018181C)
                           : ui::Color::fromArgb(0x80F2F2F7);
    setBackgroundColor(winBg);
    setBackgroundBlur({0, 0, size().width, size().height}, 20.0);
}

void MainWindow::openPhoto(const PhotoItem& photo)
{
    if (m_rootView) {
        m_rootView->openPhoto(photo);
    }
}

void MainWindow::openDirectory(const std::string& dirPath, const std::string& initialFilePath)
{
    if (m_rootView) {
        m_rootView->openDirectory(dirPath, initialFilePath);
    }
}

} // namespace cutefish::photos
