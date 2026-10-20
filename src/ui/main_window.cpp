// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/main_window.h"
#include "ui/painter.h"

namespace cutefish::photos {

PhotosRootView::PhotosRootView()
{
    m_header = addChild<ui::WindowHeader>(52.0);
    m_headerBar = m_header->setContent<HeaderBar>();
    m_sidebar = addChild<SidebarView>();
    m_galleryView = addChild<GalleryView>();

    m_sidebar->categorySelected.connect([this](NavCategory cat) {
        PhotoStore::instance().loadPhotos(cat);
    });

    m_galleryView->photoClicked.connect([this](const PhotoItem& photo) {
        showViewer(photo);
    });
}

PhotosRootView::~PhotosRootView()
{
}

void PhotosRootView::showGallery()
{
    if (m_imageViewer) {
        takeChild(m_imageViewer);
        m_imageViewer = nullptr;
    }
    if (!m_galleryView) {
        m_galleryView = addChild<GalleryView>();
        m_galleryView->photoClicked.connect([this](const PhotoItem& photo) {
            showViewer(photo);
        });
    }
    requestLayout();
    requestPaint();
}

void PhotosRootView::showViewer(const PhotoItem& photo)
{
    if (m_galleryView) {
        takeChild(m_galleryView);
        m_galleryView = nullptr;
    }
    if (m_imageViewer) {
        takeChild(m_imageViewer);
        m_imageViewer = nullptr;
    }

    m_imageViewer = addChild<ImageViewer>(photo);
    m_imageViewer->closeRequested.connect([this]() {
        showGallery();
    });
    requestLayout();
    requestPaint();
}

void PhotosRootView::layout()
{
    double w = geometry().width;
    double h = geometry().height;

    double sidebarWidth = 220.0;
    double headerHeight = 52.0;

    if (m_header) {
        m_header->setGeometry(ui::RectF(0, 0, w, headerHeight));
    }

    if (m_sidebar) {
        m_sidebar->setGeometry(ui::RectF(0, headerHeight, sidebarWidth, std::max(0.0, h - headerHeight)));
    }

    if (m_galleryView) {
        m_galleryView->setGeometry(ui::RectF(sidebarWidth, headerHeight, w - sidebarWidth, std::max(0.0, h - headerHeight)));
    }

    if (m_imageViewer) {
        m_imageViewer->setGeometry(ui::RectF(0, 0, w, h));
    }
}

void PhotosRootView::paint(ui::Painter& painter)
{
    ui::RectF bounds = localRect();
    painter.fillRect(bounds, ui::Color(0x12, 0x12, 0x16));
}

MainWindow::MainWindow()
{
    setTitle("Photos");
    setMinimumSize(ui::SizeF(800, 600));
    resize(ui::SizeF(1100, 720));

    auto rootView = std::make_unique<PhotosRootView>();
    m_rootView = rootView.get();
    setContentView(std::move(rootView));
}

MainWindow::~MainWindow()
{
}

} // namespace cutefish::photos
