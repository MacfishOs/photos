// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/views/image_viewer.h"
#include "ui/painter.h"

namespace cutefish::photos {

ImageViewer::ImageViewer(const PhotoItem& photo)
    : m_photo(photo)
{
    m_titleLabel = addChild<ui::Label>(m_photo.title + " (" + m_photo.date + ")");
    m_titleLabel->setFontSize(16.0);
    m_titleLabel->setWeight(700);
    m_titleLabel->setColor(ui::Color(0xf5, 0xf5, 0xf7));

    m_backLabel = addChild<ui::Label>("Back");
    m_backLabel->setFontSize(13.0);
    m_backLabel->setWeight(600);
    m_backLabel->setColor(ui::Color(0xff, 0xff, 0xff));
    m_backLabel->setTextAlignment(ui::Label::TextAlignment::Center);
}

ImageViewer::~ImageViewer()
{
}

void ImageViewer::layout()
{
    double w = geometry().width;
    double h = geometry().height;

    if (m_titleLabel) {
        m_titleLabel->setGeometry(ui::RectF(24, 24, w - 48, 28));
    }

    if (m_backLabel) {
        m_backLabel->setGeometry(ui::RectF(24, h - 65 + 10, 80, 20));
    }
}

void ImageViewer::paint(ui::Painter& painter)
{
    ui::RectF bounds = localRect();
    painter.fillRect(bounds, ui::Color(0x0a, 0x0a, 0x0e));

    // Bottom toolbar
    ui::RectF toolbarRect((bounds.width - 320) / 2.0, bounds.height - 70, 320, 50);
    painter.fillRoundedRect(toolbarRect, 25.0, ui::Color(0x24, 0x24, 0x2a, 0xf0));

    // Back button pill at bottom left
    ui::RectF backRect(24, bounds.height - 65, 80, 40);
    painter.fillRoundedRect(backRect, 8.0, ui::Color(0xff, 0xff, 0xff, 0x20));
}

void ImageViewer::pointerPress(const ui::PointerEvent& event)
{
    ui::RectF bounds = localRect();
    ui::RectF backRect(24, bounds.height - 65, 80, 40);
    if (backRect.contains(event.position)) {
        closeRequested.emit();
    }
}

} // namespace cutefish::photos
