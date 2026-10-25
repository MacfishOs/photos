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
    setTracksHover(true);
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
        if (m_selectedIndex >= static_cast<int>(m_photos.size())) {
            m_selectedIndex = -1;
        }
        m_firstVisible = -1;
        m_lastVisible = -1;
        updateVisibleThumbnails();
        requestLayout();
        requestPaint();
    }
}

void FilmstripCarouselView::setSelectedIndex(int index)
{
    m_selectedIndex = index;
    centerOnIndex(index, true);
    updateVisibleThumbnails();
    requestPaint();
}

double FilmstripCarouselView::maxScroll() const
{
    double w = localRect().width;
    if (w <= 0 || m_photos.empty()) return 0.0;
    double thumbSize = 52.0;
    double spacing = 8.0;
    double step = thumbSize + spacing;
    double totalW = 16.0 + m_photos.size() * step - spacing + 16.0;
    return std::max(0.0, totalW - w);
}

void FilmstripCarouselView::centerOnIndex(int index, bool animated)
{
    double w = localRect().width;
    if (w <= 0 || index < 0 || index >= static_cast<int>(m_photos.size())) return;

    double thumbSize = 52.0;
    double spacing = 8.0;
    double step = thumbSize + spacing;
    double max = maxScroll();
    double targetOffset = 0.0;

    if (max > 0.0) {
        double centerOffset = (w - thumbSize) / 2.0;
        double selectedX = 16.0 + index * step;
        targetOffset = std::clamp(selectedX - centerOffset, 0.0, max);
    }

    if (animated && window() && std::abs(m_scrollOffset - targetOffset) > 1.0) {
        if (m_scrollAnim && m_scrollAnim->isRunning()) {
            m_scrollAnim->stop();
        }
        m_animStartOffset = m_scrollOffset;
        m_animTargetOffset = targetOffset;
        m_scrollAnim = std::make_unique<ui::Animation>(window());
        m_scrollAnim->setDuration(std::chrono::milliseconds(220));
        m_scrollAnim->setEasing(ui::Animation::Easing::OutCubic);
        m_scrollAnim->onUpdate = [this](double val) {
            m_scrollOffset = m_animStartOffset + (m_animTargetOffset - m_animStartOffset) * val;
            updateVisibleThumbnails();
            requestPaint();
        };
        m_scrollAnim->start();
    } else {
        if (m_scrollAnim && m_scrollAnim->isRunning()) {
            m_scrollAnim->stop();
        }
        m_scrollOffset = targetOffset;
        updateVisibleThumbnails();
        requestPaint();
    }
}

void FilmstripCarouselView::updateVisibleThumbnails()
{
    double w = localRect().width;
    double h = localRect().height;
    if (w <= 0 || h <= 0) return;

    if (m_photos.empty()) {
        for (auto v : m_viewPool) {
            v->setVisible(false);
            v->setGeometry({-500, -500, 0, 0});
        }
        m_firstVisible = -1;
        m_lastVisible = -1;
        return;
    }

    double thumbSize = 52.0;
    double spacing = 8.0;
    double step = thumbSize + spacing;
    double leftPadding = 16.0;

    double max = maxScroll();
    m_scrollOffset = std::clamp(m_scrollOffset, 0.0, max);

    int first = static_cast<int>(std::floor((m_scrollOffset - leftPadding - step) / step));
    int last = static_cast<int>(std::ceil((m_scrollOffset + w - leftPadding + step) / step));

    first = std::clamp(first, 0, static_cast<int>(m_photos.size()) - 1);
    last = std::clamp(last, 0, static_cast<int>(m_photos.size()) - 1);

    m_firstVisible = first;
    m_lastVisible = last;

    int needed = last - first + 1;
    while (m_viewPool.size() < static_cast<size_t>(needed)) {
        auto view = addChild<ui::ImageView>();
        view->setThumbnail(true);
        view->setSourceSize({thumbSize, thumbSize});
        view->setFillMode(ui::ImageView::FillMode::PreserveAspectCrop);
        view->setCornerRadius(6.0);
        m_viewPool.push_back(view);
    }

    double y = (h - thumbSize) / 2.0;

    for (int i = first; i <= last; ++i) {
        size_t poolIdx = static_cast<size_t>(i - first);
        auto view = m_viewPool[poolIdx];
        view->setVisible(true);
        if (view->source() != m_photos[i].path) {
            view->setSource(m_photos[i].path);
        }
        view->setCornerRadius(i == m_selectedIndex ? 8.0 : 6.0);
        double x = leftPadding + i * step - m_scrollOffset;
        view->setGeometry({x, y, thumbSize, thumbSize});
    }

    for (size_t poolIdx = static_cast<size_t>(needed); poolIdx < m_viewPool.size(); ++poolIdx) {
        m_viewPool[poolIdx]->setVisible(false);
        m_viewPool[poolIdx]->setGeometry({-500, -500, 0, 0});
    }
}

void FilmstripCarouselView::layout()
{
    updateVisibleThumbnails();
}

bool FilmstripCarouselView::scroll(const ui::ScrollEvent& event)
{
    if (m_photos.empty()) return false;
    if (m_scrollAnim && m_scrollAnim->isRunning()) {
        m_scrollAnim->stop();
    }
    double max = maxScroll();
    if (max <= 0.0) return false;

    double delta = (std::abs(event.deltaX) > 0.01) ? event.deltaX : event.deltaY;
    m_scrollOffset = std::clamp(m_scrollOffset + delta, 0.0, max);
    updateVisibleThumbnails();
    requestPaint();
    return true;
}

void FilmstripCarouselView::pointerPress(const ui::PointerEvent& event)
{
    if (m_scrollAnim && m_scrollAnim->isRunning()) {
        m_scrollAnim->stop();
    }
    m_pointerDown = true;
    m_isDragging = false;
    m_dragStartPos = event.position;
    m_dragStartOffset = m_scrollOffset;
}

void FilmstripCarouselView::pointerMove(const ui::PointerEvent& event)
{
    if (!m_pointerDown) return;
    double dx = event.position.x - m_dragStartPos.x;
    if (std::abs(dx) > 4.0) {
        m_isDragging = true;
    }
    if (m_isDragging) {
        double max = maxScroll();
        m_scrollOffset = std::clamp(m_dragStartOffset - dx, 0.0, max);
        updateVisibleThumbnails();
        requestPaint();
    }
}

void FilmstripCarouselView::pointerRelease(const ui::PointerEvent& event)
{
    if (!m_pointerDown) return;
    m_pointerDown = false;

    if (!m_isDragging && !m_photos.empty()) {
        double thumbSize = 52.0;
        double spacing = 8.0;
        double step = thumbSize + spacing;
        double leftPadding = 16.0;

        double relX = event.position.x + m_scrollOffset - leftPadding;
        if (relX >= 0) {
            int idx = static_cast<int>(relX / step);
            double itemX = relX - idx * step;
            if (idx >= 0 && idx < static_cast<int>(m_photos.size()) && itemX <= thumbSize) {
                m_selectedIndex = idx;
                photoSelected.emit(idx);
                centerOnIndex(idx, true);
                requestPaint();
            }
        }
    }
    m_isDragging = false;
}

void FilmstripCarouselView::hoverMove(const ui::PointerEvent& event)
{
    double thumbSize = 52.0;
    double spacing = 8.0;
    double step = thumbSize + spacing;
    double leftPadding = 16.0;

    int newHover = -1;
    double relX = event.position.x + m_scrollOffset - leftPadding;
    if (relX >= 0) {
        int idx = static_cast<int>(relX / step);
        double itemX = relX - idx * step;
        if (idx >= 0 && idx < static_cast<int>(m_photos.size()) && itemX <= thumbSize) {
            newHover = idx;
        }
    }

    if (m_hoveredIndex != newHover) {
        m_hoveredIndex = newHover;
        requestPaint();
    }
}

void FilmstripCarouselView::pointerLeave()
{
    if (m_hoveredIndex != -1) {
        m_hoveredIndex = -1;
        requestPaint();
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

    double h = localRect().height;
    double thumbSize = 52.0;
    double spacing = 8.0;
    double step = thumbSize + spacing;
    double leftPadding = 16.0;
    double y = (h - thumbSize) / 2.0;

    // Hover highlight ring
    if (m_hoveredIndex >= 0 && m_hoveredIndex != m_selectedIndex &&
        m_hoveredIndex < static_cast<int>(m_photos.size())) {
        double x = leftPadding + m_hoveredIndex * step - m_scrollOffset;
        if (x + thumbSize >= 0 && x <= localRect().width) {
            ui::RectF hoverRect{x - 2.0, y - 2.0, thumbSize + 4.0, thumbSize + 4.0};
            ui::Color hoverCol = dark ? ui::Color::fromArgb(0x40FFFFFF) : ui::Color::fromArgb(0x30007AFF);
            painter.strokeRoundedRect(hoverRect, 9.0, hoverCol, painter.borderWidth(1.5));
        }
    }

    // Selected thumbnail accent ring
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_photos.size())) {
        double x = leftPadding + m_selectedIndex * step - m_scrollOffset;
        if (x + thumbSize >= 0 && x <= localRect().width) {
            ui::RectF selRect{x - 3.0, y - 3.0, thumbSize + 6.0, thumbSize + 6.0};
            ui::Color selColor = dark ? ui::Color::fromArgb(0xFF0A84FF)
                                      : ui::Color::fromArgb(0xFF007AFF);
            painter.strokeRoundedRect(selRect, 10.0, selColor, painter.borderWidth(2.5));
        }
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
    m_backBtn->setToolTip("Back to Library (Esc)");
    m_backBtn->onClicked = [this]() { backClicked.emit(); };

    m_titleLabel = addChild<ui::Label>("Image Viewer");
    m_titleLabel->setFontSize(13.0);
    m_titleLabel->setWeight(600);

    m_subtitleLabel = addChild<ui::Label>("");
    m_subtitleLabel->setFontSize(11.0);

    m_zoomOutBtn = addChild<ui::RoundImageButton>("apple-zoom-out", 30);
    m_zoomOutBtn->setIconSize(16);
    m_zoomOutBtn->setToolTip("Zoom Out (-)");
    m_zoomOutBtn->onClicked = [this]() { zoomOutClicked.emit(); };

    m_zoomInBtn = addChild<ui::RoundImageButton>("apple-zoom-in", 30);
    m_zoomInBtn->setIconSize(16);
    m_zoomInBtn->setToolTip("Zoom In (+)");
    m_zoomInBtn->onClicked = [this]() { zoomInClicked.emit(); };

    m_rotateLeftBtn = addChild<ui::RoundImageButton>("apple-rotate-left", 30);
    m_rotateLeftBtn->setIconSize(16);
    m_rotateLeftBtn->setToolTip("Rotate Left (L)");
    m_rotateLeftBtn->onClicked = [this]() { rotateLeftClicked.emit(); };

    m_rotateRightBtn = addChild<ui::RoundImageButton>("apple-rotate-right", 30);
    m_rotateRightBtn->setIconSize(16);
    m_rotateRightBtn->setToolTip("Rotate Right (R)");
    m_rotateRightBtn->onClicked = [this]() { rotateRightClicked.emit(); };

    m_wallpaperBtn = addChild<ui::RoundImageButton>("apple-wallpaper", 30);
    m_wallpaperBtn->setIconSize(16);
    m_wallpaperBtn->setToolTip("Set as Desktop Wallpaper");
    m_wallpaperBtn->onClicked = [this]() { wallpaperClicked.emit(); };

    m_favoriteBtn = addChild<ui::RoundImageButton>("apple-heart", 30);
    m_favoriteBtn->setIconSize(16);
    m_favoriteBtn->setToolTip("Favorite (F)");
    m_favoriteBtn->onClicked = [this]() { favoriteClicked.emit(); };

    m_infoBtn = addChild<ui::RoundImageButton>("apple-info", 30);
    m_infoBtn->setIconSize(16);
    m_infoBtn->setToolTip("Info (I)");
    m_infoBtn->onClicked = [this]() { infoClicked.emit(); };

    m_shareBtn = addChild<ui::RoundImageButton>("apple-share", 30);
    m_shareBtn->setIconSize(16);
    m_shareBtn->setToolTip("Share");
    m_shareBtn->onClicked = [this]() { shareClicked.emit(); };

    m_trashBtn = addChild<ui::RoundImageButton>("apple-trash", 30);
    m_trashBtn->setIconSize(16);
    m_trashBtn->setToolTip("Delete");
    m_trashBtn->onClicked = [this]() { trashClicked.emit(); };
}

void ImageViewerHeaderBar::setTitleInfo(const std::string& title, const std::string& subtitle)
{
    if (m_titleLabel) m_titleLabel->setText(title);
    if (m_subtitleLabel) m_subtitleLabel->setText(subtitle);
    requestLayout();
}

void ImageViewerHeaderBar::setFavorite(bool favorite)
{
    if (!m_favoriteBtn) return;
    if (favorite) {
        m_favoriteBtn->setIconName("apple-heart-filled");
        m_favoriteBtn->setIconColor(ui::Color::fromArgb(0xFFFF2D55));
    } else {
        m_favoriteBtn->setIconName("apple-heart");
        m_favoriteBtn->setIconColor({});
    }
}

void ImageViewerHeaderBar::layout()
{
    ui::RectF bounds = localRect();
    double h = bounds.height;
    if (bounds.width <= 0 || h <= 0) return;

    double midX = bounds.width / 2.0;

    m_windowControls->setGeometry({14.0, (h - 16.0) / 2.0, 56.0, 16.0});
    m_backBtn->setGeometry({78.0, (h - 30.0) / 2.0, 30.0, 30.0});

    m_titleLabel->setGeometry(ui::RectF(midX - 160.0, 8.0, 320.0, 18.0));
    m_titleLabel->setTextAlignment(ui::Label::TextAlignment::Center);
    m_subtitleLabel->setGeometry(ui::RectF(midX - 160.0, 28.0, 320.0, 16.0));
    m_subtitleLabel->setTextAlignment(ui::Label::TextAlignment::Center);

    double rx = bounds.width - 16.0;

    auto placeBtn = [&](ui::RoundImageButton* btn) {
        if (!btn) return;
        btn->setGeometry({rx - 30.0, (h - 30.0) / 2.0, 30.0, 30.0});
        rx -= 36.0;
    };

    placeBtn(m_trashBtn);
    placeBtn(m_shareBtn);
    placeBtn(m_infoBtn);
    placeBtn(m_favoriteBtn);
    rx -= 6.0; // visual separator gap
    placeBtn(m_wallpaperBtn);
    placeBtn(m_rotateRightBtn);
    placeBtn(m_rotateLeftBtn);
    placeBtn(m_zoomInBtn);
    placeBtn(m_zoomOutBtn);
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
    m_subs.push_back(m_headerBar->zoomOutClicked.connect([this]() {
        zoomOut();
    }));
    m_subs.push_back(m_headerBar->zoomInClicked.connect([this]() {
        zoomIn();
    }));
    m_subs.push_back(m_headerBar->rotateLeftClicked.connect([this]() {
        rotateLeft();
    }));
    m_subs.push_back(m_headerBar->rotateRightClicked.connect([this]() {
        rotateRight();
    }));
    m_subs.push_back(m_headerBar->wallpaperClicked.connect([this]() {
        wallpaperSet.emit();
    }));
    m_subs.push_back(m_headerBar->favoriteClicked.connect([this]() {
        if (!m_currentPath.empty()) {
            PhotoStore::instance().toggleFavorite(m_currentPath);
            bool fav = PhotoStore::instance().isFavorite(m_currentPath);
            m_photo.isFavorite = fav;
            m_headerBar->setFavorite(fav);
            currentPhotoChanged.emit(m_photo);
        }
    }));
    m_subs.push_back(m_headerBar->infoClicked.connect([this]() {
        infoPanelToggled.emit();
    }));
    m_subs.push_back(m_headerBar->shareClicked.connect([this]() {
        shareClicked.emit();
    }));
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
    m_prevBtn->setToolTip("Previous (Left Arrow)");
    m_prevBtn->onClicked = [this]() { prevClicked.emit(); };

    m_nextBtn = addChild<ui::RoundImageButton>("apple-chevron-right", 40);
    m_nextBtn->setIconSize(20);
    m_nextBtn->setBackgroundColor(ui::Color::fromArgb(0x60000000));
    m_nextBtn->setHoveredColor(ui::Color::fromArgb(0x90000000));
    m_nextBtn->setPressedColor(ui::Color::fromArgb(0xB0000000));
    m_nextBtn->setIconColor(ui::Color::fromArgb(0xFFFFFFFF));
    m_nextBtn->setToolTip("Next (Right Arrow)");
    m_nextBtn->onClicked = [this]() { nextClicked.emit(); };

    m_filmstrip = addChild<FilmstripCarouselView>();
    m_subs.push_back(m_filmstrip->photoSelected.connect([this](int index) {
        auto photos = PhotoStore::instance().photos();
        if (index >= 0 && index < static_cast<int>(photos.size())) {
            setPhoto(photos[index], index, static_cast<int>(photos.size()));
            currentPhotoChanged.emit(photos[index]);
        }
    }));
}

ImageViewer::~ImageViewer()
{
    m_decodeJob.cancel();
}

void ImageViewer::loadPhotoAsync()
{
    m_decodeJob.cancel();

    if (m_currentPath.empty()) {
        m_decodedImage = nullptr;
        m_thumbnailImage = nullptr;
        requestPaint();
        return;
    }

    // 1. Immediately request fast cached thumbnail (<1ms) for instantaneous view
    m_thumbnailImage = ui::thumbnails::request(m_currentPath, 1024, 1024);
    if (!m_thumbnailImage) {
        m_thumbnailImage = ui::thumbnails::request(m_currentPath, 256, 256);
    }
    m_decodedImage = nullptr;
    requestPaint();

    // 2. Decode pristine full-resolution image on background worker thread
    m_decodeJob = base::runInThread([path = m_currentPath]() -> sk_sp<SkImage> {
        ui::ImageRequest req;
        req.path = path;
        req.autoTransform = true;
        ui::DecodedImage decoded = ui::decodeImage(req);
        return decoded.image;
    }, [this, path = m_currentPath](sk_sp<SkImage> fullImage) {
        if (m_currentPath == path && fullImage) {
            m_decodedImage = fullImage;
            m_photo.width = fullImage->width();
            m_photo.height = fullImage->height();

            std::string subTitle = std::to_string(m_photo.width) + " × " + std::to_string(m_photo.height) +
                                   " • " + m_photo.formattedSize() +
                                   " • " + std::to_string(m_currentIndex + 1) + " of " + std::to_string(m_totalCount);
            m_headerBar->setTitleInfo(m_photo.title, subTitle);
            requestPaint();
        }
    });
}

void ImageViewer::setPhoto(const PhotoItem& photo, int index, int totalCount)
{
    m_photo = photo;
    m_currentPath = photo.path;
    m_currentIndex = index;
    m_totalCount = totalCount;

    m_zoomLevel = 1.0;
    m_rotationAngle = 0;
    m_panOffset = {0.0, 0.0};
    m_isPanning = false;

    std::string dimStr = (photo.width > 0 && photo.height > 0)
        ? (std::to_string(photo.width) + " × " + std::to_string(photo.height))
        : photo.formattedSize();
    std::string subTitle = dimStr + " • " + std::to_string(index + 1) + " of " + std::to_string(totalCount);
    m_headerBar->setTitleInfo(photo.title, subTitle);
    m_headerBar->setFavorite(PhotoStore::instance().isFavorite(photo.path));

    auto photos = PhotoStore::instance().photos();
    m_filmstrip->setPhotos(photos);
    m_filmstrip->setSelectedIndex(index);

    loadPhotoAsync();
    requestLayout();
    requestPaint();
}

void ImageViewer::pointerPress(const ui::PointerEvent& event)
{
    if (event.clickCount == 2) {
        if (m_zoomLevel > 1.05) {
            resetZoom();
        } else {
            m_zoomLevel = 2.0;
            requestPaint();
        }
        return;
    }

    if (m_zoomLevel > 1.05) {
        m_isPanning = true;
        m_panStartMouse = event.position;
        m_panStartOffset = m_panOffset;
    }
}

void ImageViewer::pointerMove(const ui::PointerEvent& event)
{
    if (m_isPanning && m_zoomLevel > 1.05) {
        m_panOffset = m_panStartOffset + (event.position - m_panStartMouse);
        requestPaint();
    }
}

void ImageViewer::pointerRelease(const ui::PointerEvent& event)
{
    (void)event;
    m_isPanning = false;
}

bool ImageViewer::scroll(const ui::ScrollEvent& event)
{
    if (event.modifiers & ui::ControlModifier) {
        if (event.deltaY < 0) {
            zoomIn();
        } else if (event.deltaY > 0) {
            zoomOut();
        }
        return true;
    }

    if (m_zoomLevel > 1.05) {
        m_panOffset.x -= event.deltaX;
        m_panOffset.y -= event.deltaY;
        requestPaint();
        return true;
    }

    if (event.deltaX > 30.0 || event.deltaY > 30.0) {
        nextClicked.emit();
        return true;
    } else if (event.deltaX < -30.0 || event.deltaY < -30.0) {
        prevClicked.emit();
        return true;
    }

    return false;
}

void ImageViewer::zoomIn()
{
    m_zoomLevel = std::min(5.0, m_zoomLevel * 1.25);
    requestPaint();
}

void ImageViewer::zoomOut()
{
    m_zoomLevel = std::max(0.2, m_zoomLevel / 1.25);
    if (m_zoomLevel <= 1.0) {
        m_panOffset = {0.0, 0.0};
    }
    requestPaint();
}

void ImageViewer::resetZoom()
{
    m_zoomLevel = 1.0;
    m_panOffset = {0.0, 0.0};
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

    if (window()) {
        window()->setBackgroundBlur(ui::RectF(0, 0, w, headerH), 16.0);
        window()->setBackgroundBlur(ui::RectF(0, h - carouselHeight, w, carouselHeight), 16.0);
    }

    m_headerBar->setGeometry({0, 0, w, headerH});

    double imgH = h - headerH - carouselHeight;

    m_prevBtn->setGeometry({24, headerH + (imgH - 40) / 2.0, 40, 40});
    m_nextBtn->setGeometry({w - 64, headerH + (imgH - 40) / 2.0, 40, 40});

    m_filmstrip->setGeometry({0, h - carouselHeight, w, carouselHeight});
}

void ImageViewer::paint(ui::Painter& painter)
{
    if (!isVisible() || localRect().width <= 0) return;

    bool dark = ui::Theme::current().darkMode;

    ui::Color bg = dark ? ui::Color::fromArgb(0xFF0A0A0C) : ui::Color::fromArgb(0xFFEBEBEF);
    painter.fillRect(localRect(), bg);

    double w = localRect().width;
    double h = localRect().height;
    double headerH = 52.0;
    double carouselHeight = 72.0;
    double areaY = headerH;
    double areaH = h - headerH - carouselHeight;

    if (areaH <= 0 || w <= 0) return;

    sk_sp<SkImage> imgToDraw = m_decodedImage ? m_decodedImage : m_thumbnailImage;

    if (imgToDraw && imgToDraw.get()) {
        double origW = static_cast<double>(imgToDraw->width());
        double origH = static_cast<double>(imgToDraw->height());

        if (origW > 0 && origH > 0) {
            double effW = (m_rotationAngle == 90 || m_rotationAngle == 270) ? origH : origW;
            double effH = (m_rotationAngle == 90 || m_rotationAngle == 270) ? origW : origH;

            double scaleFit = std::min(w / effW, areaH / effH) * m_zoomLevel;
            double destW = origW * scaleFit;
            double destH = origH * scaleFit;

            double destX = (w - destW) / 2.0 + m_panOffset.x;
            double destY = areaY + (areaH - destH) / 2.0 + m_panOffset.y;

            ui::RectF targetRect(destX, destY, destW, destH);

            painter.save();
            painter.clip(ui::RectF(0, areaY, w, areaH));

            if (m_rotationAngle != 0) {
                painter.rotate(m_rotationAngle, {w / 2.0 + m_panOffset.x, areaY + areaH / 2.0 + m_panOffset.y});
            }

            painter.drawImage(imgToDraw.get(), targetRect);
            painter.restore();
        }
    } else {
        ui::RectF loadRect{w / 2.0 - 50.0, areaY + areaH / 2.0 - 15.0, 100.0, 30.0};
        ui::Color loadBg = dark ? ui::Color::fromArgb(0x30FFFFFF) : ui::Color::fromArgb(0x30000000);
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
    if (event.key == letter('l') || event.key == '[' || event.key == ']') {
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
            bool fav = PhotoStore::instance().isFavorite(m_currentPath);
            m_photo.isFavorite = fav;
            m_headerBar->setFavorite(fav);
            currentPhotoChanged.emit(m_photo);
        }
        return true;
    }
    if (event.key == '+' || event.key == '=') {
        zoomIn();
        return true;
    }
    if (event.key == '-' || event.key == '_') {
        zoomOut();
        return true;
    }
    if (event.key == '0') {
        resetZoom();
        return true;
    }

    return ui::View::keyPress(event);
}

} // namespace cutefish::photos
