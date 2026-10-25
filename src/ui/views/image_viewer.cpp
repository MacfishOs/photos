// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/views/image_viewer.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

namespace cutefish::photos {

// ============================================================================
// FilmstripCarouselView
// ============================================================================

FilmstripCarouselView::FilmstripCarouselView()
{
    setClipsChildren(true);
}

void FilmstripCarouselView::setPhotos(const std::vector<PhotoItem>& photos)
{
    bool changed = (m_photos.size() != photos.size());
    if (!changed) {
        for (size_t i = 0; i < photos.size(); ++i) {
            if (m_photos[i].path != photos[i].path) {
                changed = true;
                break;
            }
        }
    }
    if (changed) {
        m_photos = photos;
        rebuildThumbnails();
    }
}

void FilmstripCarouselView::setSelectedIndex(int index)
{
    m_selectedIndex = index;
    for (size_t i = 0; i < m_thumbs.size(); ++i) {
        m_thumbs[i]->setCornerRadius((static_cast<int>(i) == index) ? 8.0 : 6.0);
    }
    requestLayout();
    requestPaint();
}

double FilmstripCarouselView::scrollOffset() const
{
    double h = localRect().height;
    double w = localRect().width;
    if (h <= 0 || w <= 0 || m_thumbs.empty()) return 0.0;

    double thumbSize = 52.0;
    double step = thumbSize + 8.0;
    double totalW = m_thumbs.size() * step + 24.0;
    if (totalW <= w) return 0.0;

    double centerOffset = (w - thumbSize) / 2.0;
    double selectedX = m_selectedIndex * step + 16.0;
    double scrollX = selectedX - centerOffset;
    return std::clamp(scrollX, 0.0, totalW - w);
}

void FilmstripCarouselView::rebuildThumbnails()
{
    for (auto t : m_thumbs) {
        auto old = takeChild(t);
    }
    m_thumbs.clear();

    for (size_t i = 0; i < m_photos.size(); ++i) {
        auto thumb = addChild<ui::ImageView>(m_photos[i].path);
        thumb->setCornerRadius(6.0);
        thumb->setFillMode(ui::ImageView::FillMode::PreserveAspectCrop);
        m_thumbs.push_back(thumb);
    }

    requestLayout();
}

void FilmstripCarouselView::layout()
{
    double h = localRect().height;
    if (h <= 0 || !isVisible()) return;

    double thumbSize = 52.0;
    double offset = scrollOffset();
    double x = 16.0 - offset;
    double y = (h - thumbSize) / 2.0;

    for (size_t i = 0; i < m_thumbs.size(); ++i) {
        m_thumbs[i]->setGeometry({x, y, thumbSize, thumbSize});
        x += thumbSize + 8.0;
    }
}

void FilmstripCarouselView::pointerPress(const ui::PointerEvent& event)
{
    double h = localRect().height;
    double thumbSize = 52.0;
    double step = thumbSize + 8.0;
    double offset = scrollOffset();
    double x = 16.0 - offset;
    double y = (h - thumbSize) / 2.0;

    for (size_t i = 0; i < m_thumbs.size(); ++i) {
        ui::RectF rect{x, y, thumbSize, thumbSize};
        if (rect.contains(event.position)) {
            m_selectedIndex = static_cast<int>(i);
            photoSelected.emit(static_cast<int>(i));
            setSelectedIndex(static_cast<int>(i));
            break;
        }
        x += step;
    }
}

void FilmstripCarouselView::paint(ui::Painter& painter)
{
    if (!isVisible() || localRect().width <= 0) return;

    bool dark = ui::Theme::current().darkMode;

    // Translucent Apple Photos frosted acrylic backdrop
    ui::Color bg = dark ? ui::Color::fromArgb(0xC8141418)
                        : ui::Color::fromArgb(0xC8F4F4F6);
    painter.fillRect(localRect(), bg);

    ui::Color border = dark ? ui::Color::fromArgb(0x28FFFFFF) : ui::Color::fromArgb(0x18000000);
    painter.drawHorizontalLine(0, localRect().width, 0, border);

    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_thumbs.size())) {
        double h = localRect().height;
        double thumbSize = 52.0;
        double step = thumbSize + 8.0;
        double offset = scrollOffset();
        double x = 16.0 + m_selectedIndex * step - offset;
        double y = (h - thumbSize) / 2.0;

        ui::RectF selRect{x - 3.0, y - 3.0, thumbSize + 6.0, thumbSize + 6.0};
        ui::Color selColor = dark ? ui::Color::fromArgb(0xFF007AFF)
                                  : ui::Color::fromArgb(0xFF0066CC);
        painter.strokeRoundedRect(selRect, 10.0, selColor, painter.borderWidth(2.5));
    }
}

// ============================================================================
// ImageViewerHeaderBar
// ============================================================================

ImageViewerHeaderBar::ImageViewerHeaderBar()
{
    m_windowControls = addChild<WindowControls>();

    m_backBtn = addChild<ui::RoundImageButton>("apple-chevron-left", 30);
    m_backBtn->setIconSize(16);
    m_backBtn->onClicked = [this]() { backClicked.emit(); };

    m_titleLabel = addChild<ui::Label>("Image Viewer");
    m_titleLabel->setFontSize(13.0);
    m_titleLabel->setWeight(600);

    m_subtitleLabel = addChild<ui::Label>("macOS Photos");
    m_subtitleLabel->setFontSize(11.0);

    m_favoriteBtn = addChild<ui::RoundImageButton>("apple-heart", 30);
    m_favoriteBtn->setIconSize(16);
    m_favoriteBtn->onClicked = [this]() { favoriteClicked.emit(); };

    m_infoBtn = addChild<ui::RoundImageButton>("apple-info", 30);
    m_infoBtn->setIconSize(16);
    m_infoBtn->onClicked = [this]() { infoClicked.emit(); };

    m_shareBtn = addChild<ui::RoundImageButton>("apple-share", 30);
    m_shareBtn->setIconSize(16);
    m_shareBtn->onClicked = [this]() { shareClicked.emit(); };

    m_trashBtn = addChild<ui::RoundImageButton>("apple-trash", 30);
    m_trashBtn->setIconSize(16);
    m_trashBtn->onClicked = [this]() { trashClicked.emit(); };
}

void ImageViewerHeaderBar::setTitleInfo(const std::string& title, const std::string& subtitle)
{
    if (m_titleLabel) m_titleLabel->setText(title);
    if (m_subtitleLabel) m_subtitleLabel->setText(subtitle);
    requestLayout();
}

void ImageViewerHeaderBar::layout()
{
    ui::RectF bounds = localRect();
    double h = bounds.height;
    if (bounds.width <= 0 || h <= 0) return;

    double midX = bounds.width / 2.0;

    m_windowControls->setGeometry({14.0, (h - 16.0) / 2.0, 56.0, 16.0});
    m_backBtn->setGeometry({78.0, (h - 30.0) / 2.0, 30.0, 30.0});

    m_titleLabel->setGeometry(ui::RectF(midX - 180.0, 8.0, 360.0, 18.0));
    m_subtitleLabel->setGeometry(ui::RectF(midX - 180.0, 28.0, 360.0, 16.0));

    double rx = bounds.width - 16.0;

    m_trashBtn->setGeometry({rx - 30.0, (h - 30.0) / 2.0, 30.0, 30.0});
    rx -= 38.0;

    m_shareBtn->setGeometry({rx - 30.0, (h - 30.0) / 2.0, 30.0, 30.0});
    rx -= 38.0;

    m_infoBtn->setGeometry({rx - 30.0, (h - 30.0) / 2.0, 30.0, 30.0});
    rx -= 38.0;

    m_favoriteBtn->setGeometry({rx - 30.0, (h - 30.0) / 2.0, 30.0, 30.0});
}

void ImageViewerHeaderBar::paint(ui::Painter& painter)
{
    ui::RectF bounds = localRect();
    bool dark = ui::Theme::current().darkMode;

    // Translucent Header Bar Background - Apple Photos Frosted Glass
    painter.fillRect(bounds, dark ? ui::Color::fromArgb(0xC818181C) : ui::Color::fromArgb(0xC8F4F4F6));
    painter.drawHorizontalLine(0, bounds.width, bounds.height - 1, dark ? ui::Color::fromArgb(0x28FFFFFF) : ui::Color::fromArgb(0x18000000));

    ui::Color fgColor = dark ? ui::Color::fromArgb(0xFFF5F5F7) : ui::Color::fromArgb(0xFF1D1D1F);
    if (m_titleLabel) m_titleLabel->setColor(fgColor);
    if (m_subtitleLabel) m_subtitleLabel->setColor(dark ? ui::Color::fromArgb(0xFF98989D) : ui::Color::fromArgb(0xFF8E8E93));
}

void ImageViewerHeaderBar::pointerPress(const ui::PointerEvent& event)
{
    (void)event;
    if (window()) {
        window()->startMove();
    }
}

// ============================================================================
// ImageViewer
// ============================================================================

ImageViewer::ImageViewer()
{
    m_headerBar = addChild<ImageViewerHeaderBar>();
    m_subs.push_back(m_headerBar->backClicked.connect([this]() {
        closeRequested.emit();
    }));
    m_subs.push_back(m_headerBar->favoriteClicked.connect([this]() {
        if (!m_currentPath.empty()) {
            PhotoStore::instance().toggleFavorite(m_currentPath);
        }
    }));
    m_subs.push_back(m_headerBar->infoClicked.connect([this]() { infoPanelToggled.emit(); }));
    m_subs.push_back(m_headerBar->shareClicked.connect([this]() { shareClicked.emit(); }));
    m_subs.push_back(m_headerBar->trashClicked.connect([this]() {
        if (!m_currentPath.empty()) {
            PhotoStore::instance().moveToTrash(m_currentPath);
            closeRequested.emit();
        }
    }));

    m_prevBtn = addChild<ui::RoundImageButton>("apple-chevron-left", 40);
    m_prevBtn->setIconSize(20);
    m_prevBtn->setBackgroundColor(ui::Color::fromArgb(0x60000000));
    m_prevBtn->setHoveredColor(ui::Color::fromArgb(0x90000000));
    m_prevBtn->setPressedColor(ui::Color::fromArgb(0xB0000000));
    m_prevBtn->setIconColor(ui::Color::fromArgb(0xFFFFFFFF));
    m_prevBtn->onClicked = [this]() { prevClicked.emit(); };

    m_nextBtn = addChild<ui::RoundImageButton>("apple-chevron-right", 40);
    m_nextBtn->setIconSize(20);
    m_nextBtn->setBackgroundColor(ui::Color::fromArgb(0x60000000));
    m_nextBtn->setHoveredColor(ui::Color::fromArgb(0x90000000));
    m_nextBtn->setPressedColor(ui::Color::fromArgb(0xB0000000));
    m_nextBtn->setIconColor(ui::Color::fromArgb(0xFFFFFFFF));
    m_nextBtn->onClicked = [this]() { nextClicked.emit(); };

    m_filmstrip = addChild<FilmstripCarouselView>();
    m_subs.push_back(m_filmstrip->photoSelected.connect([this](int index) {
        auto photos = PhotoStore::instance().photos();
        if (index >= 0 && index < static_cast<int>(photos.size())) {
            setPhoto(photos[index], index, static_cast<int>(photos.size()));
        }
    }));
}

void ImageViewer::reloadDecodedImage()
{
    if (m_currentPath.empty()) {
        m_decodedImage = nullptr;
        return;
    }

    ui::ImageRequest req;
    req.path = m_currentPath;
    req.autoTransform = true;

    ui::DecodedImage decoded = ui::decodeImage(req);
    m_decodedImage = decoded.image;
    requestPaint();
}

void ImageViewer::setPhoto(const PhotoItem& photo, int index, int totalCount)
{
    m_photo = photo;
    m_currentPath = photo.path;
    m_currentIndex = index;
    m_totalCount = totalCount;

    m_zoomLevel = 1.0;
    m_rotationAngle = 0;

    std::string subTitle = photo.formattedDimensions() + " • " + std::to_string(index + 1) + " of " + std::to_string(totalCount);
    m_headerBar->setTitleInfo(photo.title, subTitle);

    auto photos = PhotoStore::instance().photos();
    m_filmstrip->setPhotos(photos);
    m_filmstrip->setSelectedIndex(index);

    reloadDecodedImage();
    requestLayout();
    requestPaint();
}

void ImageViewer::pointerPress(const ui::PointerEvent& event)
{
    (void)event;
}

void ImageViewer::zoomIn()
{
    m_zoomLevel *= 1.25;
    if (m_zoomLevel > 5.0) m_zoomLevel = 5.0;
    requestPaint();
}

void ImageViewer::zoomOut()
{
    m_zoomLevel /= 1.25;
    if (m_zoomLevel < 0.2) m_zoomLevel = 0.2;
    requestPaint();
}

void ImageViewer::rotateLeft()
{
    m_rotationAngle = (m_rotationAngle - 90 + 360) % 360;
    requestPaint();
}

void ImageViewer::rotateRight()
{
    m_rotationAngle = (m_rotationAngle + 90) % 360;
    requestPaint();
}

void ImageViewer::layout()
{
    double w = localRect().width;
    double h = localRect().height;

    if (w <= 0 || h <= 0) return;

    double headerH = 52.0;
    double carouselHeight = 72.0;

    // Apply acrylic glass blur to top header bar and bottom filmstrip carousel
    if (window()) {
        window()->setBackgroundBlur(ui::RectF(0, 0, w, headerH), 16.0);
        window()->setBackgroundBlur(ui::RectF(0, h - carouselHeight, w, carouselHeight), 16.0);
    }

    m_headerBar->setGeometry({0, 0, w, headerH});

    double imgW = w;
    double imgH = h - headerH - carouselHeight;

    m_prevBtn->setGeometry({24, headerH + (imgH - 40) / 2.0, 40, 40});
    m_nextBtn->setGeometry({w - 64, headerH + (imgH - 40) / 2.0, 40, 40});

    m_filmstrip->setGeometry({0, h - carouselHeight, w, carouselHeight});
}

void ImageViewer::paint(ui::Painter& painter)
{
    if (!isVisible() || localRect().width <= 0) return;

    bool dark = ui::Theme::current().darkMode;

    // macOS Dark Image Viewer background
    ui::Color bg = dark ? ui::Color::fromArgb(0xFF0A0A0C)
                        : ui::Color::fromArgb(0xFFEBEBEF);
    painter.fillRect(localRect(), bg);

    if (m_decodedImage && m_decodedImage.get()) {
        double viewportW = localRect().width;
        double viewportH = localRect().height;

        if (viewportW > 0 && viewportH > 0) {
            double origW = static_cast<double>(m_decodedImage->width());
            double origH = static_cast<double>(m_decodedImage->height());

            if (origW > 0 && origH > 0) {
                double scaleFit = std::min(viewportW / origW, viewportH / origH) * m_zoomLevel;
                double destW = origW * scaleFit;
                double destH = origH * scaleFit;

                double destX = (viewportW - destW) / 2.0;
                double destY = (viewportH - destH) / 2.0;

                ui::RectF targetRect(destX, destY, destW, destH);

                painter.save();
                painter.drawImage(m_decodedImage.get(), targetRect);
                painter.restore();
            }
        }
    } else {
        double w = localRect().width;
        double h = localRect().height;
        ui::RectF loadRect{w / 2.0 - 50.0, h / 2.0 - 15.0, 100.0, 30.0};
        ui::Color loadBg = dark ? ui::Color::fromArgb(0x30FFFFFF)
                                : ui::Color::fromArgb(0x30000000);
        painter.fillRoundedRect(loadRect, 15.0, loadBg);
    }
}

bool ImageViewer::keyPress(const ui::KeyEvent& event)
{
    using namespace ui::key;

    if (event.key == Left || event.key == letter('a') || event.key == PageUp) {
        prevClicked.emit();
        return true;
    }
    if (event.key == Right || event.key == letter('d') || event.key == PageDown) {
        nextClicked.emit();
        return true;
    }
    if (event.key == Escape || event.key == BackSpace) {
        closeRequested.emit();
        return true;
    }
    if (event.key == letter('r')) {
        rotateRight();
        return true;
    }
    if (event.key == letter('l')) {
        rotateLeft();
        return true;
    }
    if (event.key == letter('i')) {
        infoPanelToggled.emit();
        return true;
    }
    if (event.key == letter('f')) {
        if (!m_currentPath.empty()) {
            PhotoStore::instance().toggleFavorite(m_currentPath);
        }
        return true;
    }
    if (event.key == '+' || event.key == '=') {
        zoomIn();
        return true;
    }
    if (event.key == '-') {
        zoomOut();
        return true;
    }

    return ui::View::keyPress(event);
}

} // namespace cutefish::photos
