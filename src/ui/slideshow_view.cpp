// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/slideshow_view.h"
#include "ui/painter.h"
#include "ui/theme.h"

namespace cutefish::photos {

SlideshowView::SlideshowView()
{
    m_imageView = addChild<ui::ImageView>();
    m_imageView->setFillMode(ui::ImageView::FillMode::PreserveAspectFit);
    m_imageView->setAutoTransform(true);

    // Top Header
    m_backBtn = addChild<ui::RoundImageButton>("go-previous-symbolic", 28);
    m_backBtn->onClicked = [this]() {
        stop();
        exitClicked.emit();
    };

    m_volumeSlider = addChild<ui::Slider>();
    m_volumeSlider->setValue(0.7);

    m_headerTitleLabel = addChild<ui::Label>("Oxnard Beach");
    m_headerTitleLabel->setFontSize(13);
    m_headerTitleLabel->setWeight(700);
    m_headerTitleLabel->setColor(ui::Color::rgb(0xFFFFFF));
    m_headerTitleLabel->setTextAlignment(ui::Label::TextAlignment::Center);

    m_headerSubLabel = addChild<ui::Label>("Whistler Float - Kyle White");
    m_headerSubLabel->setFontSize(10);
    m_headerSubLabel->setColor(ui::Color::rgb(0xCCCCCC));
    m_headerSubLabel->setTextAlignment(ui::Label::TextAlignment::Center);

    m_shareBtn = addChild<ui::RoundImageButton>("document-send-symbolic", 28);
    m_favoriteBtn = addChild<ui::RoundImageButton>("emblem-favorite-symbolic", 28);

    m_customizePill = addChild<ui::Label>("Customize");
    m_customizePill->setFontSize(12);
    m_customizePill->setWeight(600);
    m_customizePill->setTextAlignment(ui::Label::TextAlignment::Center);

    // Center Memory Overlay Title
    m_overlayTitleLabel = addChild<ui::Label>("At the Beach");
    m_overlayTitleLabel->setFontSize(38);
    m_overlayTitleLabel->setWeight(800);
    m_overlayTitleLabel->setColor(ui::Color::rgb(0xFFFFFF));
    m_overlayTitleLabel->setTextAlignment(ui::Label::TextAlignment::Center);

    m_overlaySubLabel = addChild<ui::Label>("OVER THE YEARS");
    m_overlaySubLabel->setFontSize(16);
    m_overlaySubLabel->setWeight(700);
    m_overlaySubLabel->setColor(ui::Color::rgb(0xFFFFFF));
    m_overlaySubLabel->setTextAlignment(ui::Label::TextAlignment::Center);

    // Bottom Floating Circular Buttons
    m_prevBtn = addChild<ui::RoundImageButton>("media-skip-backward-symbolic", 38);
    m_prevBtn->onClicked = [this]() { prevPhoto(); };

    m_musicBtn = addChild<ui::RoundImageButton>("audio-x-generic-symbolic", 38);
    m_styleBtn = addChild<ui::RoundImageButton>("preferences-desktop-theme-symbolic", 38);

    m_filmstrip = addChild<FilmstripCarouselView>();
    (void)m_filmstrip->photoSelected.connect([this](int index) {
        if (index >= 0 && index < static_cast<int>(m_photos.size())) {
            m_currentIndex = index;
            showCurrentPhoto();
        }
    });

    m_pauseBtn = addChild<ui::RoundImageButton>("media-playback-pause-symbolic", 38);
    m_pauseBtn->onClicked = [this]() {
        if (m_timer.isActive()) {
            stop();
        } else {
            m_timer.start(std::chrono::milliseconds(3000));
        }
    };

    m_gridBtn = addChild<ui::RoundImageButton>("view-grid-symbolic", 38);
    m_gridBtn->onClicked = [this]() {
        stop();
        exitClicked.emit();
    };
}

SlideshowView::~SlideshowView()
{
    stop();
}

void SlideshowView::start(const std::vector<PhotoItem>& photos, int startIndex)
{
    m_photos = photos;
    m_currentIndex = startIndex;

    if (m_photos.empty()) return;

    m_filmstrip->setPhotos(m_photos);
    showCurrentPhoto();

    m_timer.onTimeout = [this]() {
        nextPhoto();
    };
    m_timer.start(std::chrono::milliseconds(3000));
}

void SlideshowView::stop()
{
    m_timer.stop();
}

void SlideshowView::nextPhoto()
{
    if (m_photos.empty()) return;
    m_currentIndex = (m_currentIndex + 1) % m_photos.size();
    showCurrentPhoto();
}

void SlideshowView::prevPhoto()
{
    if (m_photos.empty()) return;
    m_currentIndex = (m_currentIndex - 1 + m_photos.size()) % m_photos.size();
    showCurrentPhoto();
}

void SlideshowView::showCurrentPhoto()
{
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_photos.size())) {
        const auto& photo = m_photos[m_currentIndex];
        m_imageView->setSource(photo.path);
        m_overlayTitleLabel->setText(photo.title.empty() ? "At the Beach" : photo.title);
        m_filmstrip->setSelectedIndex(m_currentIndex);
        requestPaint();
    }
}

void SlideshowView::layout()
{
    double w = localRect().width;
    double h = localRect().height;

    if (w <= 0 || h <= 0 || !isVisible()) {
        m_imageView->setGeometry({0, 0, 0, 0});
        m_backBtn->setGeometry({0, 0, 0, 0});
        m_volumeSlider->setGeometry({0, 0, 0, 0});
        m_headerTitleLabel->setGeometry({0, 0, 0, 0});
        m_headerSubLabel->setGeometry({0, 0, 0, 0});
        m_customizePill->setGeometry({0, 0, 0, 0});
        m_favoriteBtn->setGeometry({0, 0, 0, 0});
        m_shareBtn->setGeometry({0, 0, 0, 0});
        m_overlayTitleLabel->setGeometry({0, 0, 0, 0});
        m_overlaySubLabel->setGeometry({0, 0, 0, 0});
        m_prevBtn->setGeometry({0, 0, 0, 0});
        m_musicBtn->setGeometry({0, 0, 0, 0});
        m_styleBtn->setGeometry({0, 0, 0, 0});
        m_filmstrip->setGeometry({0, 0, 0, 0});
        m_pauseBtn->setGeometry({0, 0, 0, 0});
        m_gridBtn->setGeometry({0, 0, 0, 0});
        return;
    }

    m_imageView->setGeometry({0, 0, w, h});

    // Top Header Bar Layout
    m_backBtn->setGeometry({16, 12, 28, 28});
    m_volumeSlider->setGeometry({52, 16, 90, 20});

    m_headerTitleLabel->setGeometry({(w - 240) / 2.0, 8, 240, 18});
    m_headerSubLabel->setGeometry({(w - 240) / 2.0, 26, 240, 16});

    double rightX = w - 16.0;
    m_customizePill->setGeometry({rightX - 80, 10, 80, 28});
    rightX -= 90.0;

    m_favoriteBtn->setGeometry({rightX - 28, 10, 28, 28});
    rightX -= 36.0;

    m_shareBtn->setGeometry({rightX - 28, 10, 28, 28});

    // Center Memory Overlay Title
    double overlayY = h * 0.62;
    m_overlayTitleLabel->setGeometry({0, overlayY, w, 44});
    m_overlaySubLabel->setGeometry({0, overlayY + 48, w, 24});

    // Bottom Glass Circular Buttons & Filmstrip
    double botY = h - 60.0;

    m_prevBtn->setGeometry({20, botY, 38, 38});
    m_musicBtn->setGeometry({66, botY, 38, 38});
    m_styleBtn->setGeometry({112, botY, 38, 38});

    double stripW = 340.0;
    double stripX = (w - stripW) / 2.0;
    m_filmstrip->setGeometry({stripX, botY - 10, stripW, 58});

    m_pauseBtn->setGeometry({w - 108, botY, 38, 38});
    m_gridBtn->setGeometry({w - 58, botY, 38, 38});
}

void SlideshowView::paint(ui::Painter& painter)
{
    if (!isVisible() || localRect().width <= 0 || localRect().height <= 0) return;

    double w = localRect().width;
    double h = localRect().height;

    // Fullscreen backdrop
    painter.fillRect(localRect(), ui::Color::fromArgb(0xFF0A0A0C));

    // Customize Pill background
    ui::RectF custRect = m_customizePill->geometry();
    painter.fillRoundedRect(custRect, 14.0, ui::Color::fromArgb(0x40FFFFFF));

    // Bottom Floating Glass Buttons Frosted Fill
    auto drawGlassCircle = [&painter](const ui::RectF& rect) {
        if (rect.width > 0 && rect.height > 0) {
            painter.fillEllipse(rect, ui::Color::fromArgb(0x50FFFFFF));
            painter.drawShadow(rect, rect.width / 2.0, 6.0, ui::Color::fromArgb(0x30000000), {0, 2});
        }
    };

    drawGlassCircle(m_prevBtn->geometry());
    drawGlassCircle(m_musicBtn->geometry());
    drawGlassCircle(m_styleBtn->geometry());
    drawGlassCircle(m_pauseBtn->geometry());
    drawGlassCircle(m_gridBtn->geometry());
}

} // namespace cutefish::photos
