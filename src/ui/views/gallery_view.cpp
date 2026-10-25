// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/views/gallery_view.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include <iostream>

namespace cutefish::photos {

// ============================================================================
// AlbumCardView
// ============================================================================

AlbumCardView::AlbumCardView(const AlbumItem& album)
    : m_album(album)
{
    m_imageView = addChild<ClickableImageView>(album.coverPhotoPath);
    m_imageView->setCornerRadius(22.0);
    m_imageView->setFillMode(ui::ImageView::FillMode::PreserveAspectCrop);
    m_imageView->onClicked = [this]() {
        if (onClicked) onClicked();
    };

    m_titleLabel = addChild<ui::Label>(album.title);
    m_titleLabel->setFontSize(14);
    m_titleLabel->setWeight(700);
    m_titleLabel->setColor(ui::Color::rgb(0xFFFFFF));
}

void AlbumCardView::layout()
{
    double w = localRect().width;
    double h = localRect().height;

    m_imageView->setGeometry(localRect());
    m_titleLabel->setGeometry({16, h - 34, w - 32, 22});
}

void AlbumCardView::pointerPress(const ui::PointerEvent& event)
{
    (void)event;
    if (onClicked) onClicked();
}

void AlbumCardView::paint(ui::Painter& painter)
{
    ui::RectF rect = localRect();

    // Soft drop shadow around album card
    painter.drawShadow(rect, 22.0, 12.0, ui::Color::fromArgb(0x28000000), {0, 6});

    // Dark gradient overlay scrim at bottom of photo card
    ui::RectF scrimRect{0, rect.height - 60.0, rect.width, 60.0};
    ui::Color scrimGrad = ui::Color::fromArgb(0x90000000);
    painter.fillRoundedRect(scrimRect, 22.0, scrimGrad);
}

// ============================================================================
// GalleryCardView
// ============================================================================

GalleryCardView::GalleryCardView(const PhotoItem& photo, double cardWidth)
    : m_item(photo)
    , m_cardWidth(cardWidth)
{
    m_imageView = addChild<ClickableImageView>();
    m_imageView->setFillMode(ui::ImageView::FillMode::PreserveAspectCrop);
    m_imageView->setCornerRadius(8.0);
    m_imageView->onClicked = [this]() {
        if (onClicked) onClicked();
    };

    if (photo.isFavorite) {
        m_heartIcon = addChild<ui::ImageView>("emblem-favorite-symbolic");
    }
    ui::View::setVisible(false);
}

void GalleryCardView::setCardVisible(bool visible)
{
    setVisible(visible);
    if (visible && m_imageView && !m_loaded) {
        m_loaded = true;
        m_imageView->setSource(m_item.path);
    }
}

void GalleryCardView::setCardWidth(double width)
{
    m_cardWidth = width;
    requestLayout();
}

void GalleryCardView::setSelected(bool selected)
{
    m_selected = selected;
    requestPaint();
}

void GalleryCardView::pointerEnter()
{
    m_hovered = true;
    requestPaint();
}

void GalleryCardView::pointerLeave()
{
    m_hovered = false;
    requestPaint();
}

void GalleryCardView::pointerPress(const ui::PointerEvent& event)
{
    (void)event;
    if (onClicked) onClicked();
}

void GalleryCardView::layout()
{
    double h = localRect().height;
    m_imageView->setGeometry(localRect());

    if (m_heartIcon) {
        m_heartIcon->setGeometry({8, h - 24, 16, 16});
    }
}

void GalleryCardView::paint(ui::Painter& painter)
{
    ui::RectF rect = localRect();
    bool dark = ui::Theme::current().darkMode;

    if (!m_loaded) {
        ui::Color skelBg = dark ? ui::Color::fromArgb(0x18FFFFFF)
                                : ui::Color::fromArgb(0x10000000);
        painter.fillRoundedRect(rect, 8.0, skelBg);
    }

    if (m_selected) {
        ui::Color selBorder = ui::Color::rgb(0x007AFF);
        painter.strokeRoundedRect(rect, 8.0, selBorder, painter.borderWidth(3));
    } else if (m_hovered) {
        ui::Color hoverBorder = ui::Color::fromArgb(0x80007AFF);
        painter.strokeRoundedRect(rect, 8.0, hoverBorder, painter.borderWidth(2));
    }
}

// ============================================================================
// GalleryContentView
// ============================================================================

GalleryContentView::GalleryContentView()
{
    refresh();
}

void GalleryContentView::refresh()
{
    m_photos = PhotoStore::instance().photos();
    m_albums = PhotoStore::instance().albums();
    rebuildGrid();
}

void GalleryContentView::setThumbnailSize(double size)
{
    m_cardSize = size;
    requestLayout();
}

void GalleryContentView::updateVisibleViewport(ui::PointF scrollPos, double viewportHeight)
{
    m_scrollPos = scrollPos;
    m_viewportHeight = viewportHeight;
    updateCardVisibility();
}

void GalleryContentView::updateCardVisibility()
{
    if (m_photoCards.empty()) return;

    double minY = m_scrollPos.y - 300.0;
    double maxY = m_scrollPos.y + (m_viewportHeight > 0 ? m_viewportHeight : 800.0) + 300.0;

    for (auto card : m_photoCards) {
        double cardY = card->geometry().y;
        double cardH = card->geometry().height;

        bool visible = (cardY + cardH >= minY && cardY <= maxY);
        card->setCardVisible(visible);
    }
}

const PhotoItem* GalleryContentView::selectedPhoto() const
{
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_photos.size())) {
        return &m_photos[m_selectedIndex];
    }
    return nullptr;
}

void GalleryContentView::rebuildGrid()
{
    for (auto card : m_photoCards) {
        auto old = takeChild(card);
    }
    for (auto card : m_albumCards) {
        auto old = takeChild(card);
    }

    m_photoCards.clear();
    m_albumCards.clear();

    for (size_t i = 0; i < m_photos.size(); ++i) {
        const auto& photo = m_photos[i];
        auto card = addChild<GalleryCardView>(photo, m_cardSize);

        card->onClicked = [this, i, photo]() {
            m_selectedIndex = static_cast<int>(i);
            for (size_t c = 0; c < m_photoCards.size(); ++c) {
                m_photoCards[c]->setSelected(static_cast<int>(c) == m_selectedIndex);
            }
            photoClicked.emit(photo);
        };

        m_photoCards.push_back(card);
    }

    requestLayout();
}

void GalleryContentView::pointerPress(const ui::PointerEvent& event)
{
    for (size_t i = 0; i < m_photoCards.size() && i < m_photos.size(); ++i) {
        if (m_photoCards[i]->geometry().contains(event.position)) {
            m_selectedIndex = static_cast<int>(i);
            for (size_t c = 0; c < m_photoCards.size(); ++c) {
                m_photoCards[c]->setSelected(static_cast<int>(c) == m_selectedIndex);
            }
            const auto& photo = m_photos[i];
            photoClicked.emit(photo);
            return;
        }
    }
}

void GalleryContentView::layout()
{
    double availableW = localRect().width;
    if (availableW <= 0) return;

    // Photo Grid Layout
    double margin = 12.0;
    double spacing = 6.0;

    int cols = static_cast<int>((availableW - margin * 2.0 + spacing) / (m_cardSize + spacing));
    if (cols < 1) cols = 1;

    double cardW = (availableW - margin * 2.0 - (cols - 1) * spacing) / cols;
    double cardH = cardW;

    double x = margin;
    double y = margin;
    int col = 0;

    for (auto card : m_photoCards) {
        card->setCardWidth(cardW);
        card->setGeometry({x, y, cardW, cardH});

        col++;
        if (col >= cols) {
            col = 0;
            x = margin;
            y += cardH + spacing;
        } else {
            x += cardW + spacing;
        }
    }
    if (col > 0) y += cardH + spacing;
    m_totalHeight = std::max(y + margin, 200.0);

    updateCardVisibility();
}

// ============================================================================
// GalleryView
// ============================================================================

GalleryView::GalleryView()
{
    m_scrollView = addChild<ui::ScrollView>();
    auto content = std::make_unique<GalleryContentView>();
    m_content = content.get();
    m_scrollView->setContentView(std::move(content));

    m_subs.push_back(m_scrollView->scrolled.connect([this]() {
        if (m_content && m_scrollView) {
            m_content->updateVisibleViewport(m_scrollView->scrollPosition(), m_scrollView->geometry().height);
        }
    }));

    m_subs.push_back(m_content->photoClicked.connect([this](const PhotoItem& item) {
        photoClicked.emit(item);
    }));

    m_subs.push_back(m_content->photoDoubleClicked.connect([this](const PhotoItem& item) {
        photoDoubleClicked.emit(item);
    }));

    m_subs.push_back(m_content->albumClicked.connect([this](const AlbumItem& alb) {
        albumClicked.emit(alb);
    }));

    m_storeSub = PhotoStore::instance().photosChanged.connect([this]() {
        if (m_content) {
            m_content->refresh();
        }
    });
}

void GalleryView::setThumbnailSize(double size)
{
    if (m_content) {
        m_content->setThumbnailSize(size);
    }
}

void GalleryView::layout()
{
    m_scrollView->setGeometry(localRect());
    if (m_content && m_scrollView) {
        m_content->updateVisibleViewport(m_scrollView->scrollPosition(), m_scrollView->geometry().height);
    }
}

} // namespace cutefish::photos
