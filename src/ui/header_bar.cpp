// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/header_bar.h"
#include "ui/painter.h"
#include "ui/window.h"

namespace cutefish::photos {

HeaderBar::HeaderBar()
{
    m_titleLabel = addChild<ui::Label>("Photos");
    m_titleLabel->setFontSize(16.0);
    m_titleLabel->setWeight(700);
    m_titleLabel->setColor(ui::Color(0xf5, 0xf5, 0xf7));

    m_searchField = addChild<ui::SearchField>("Search photos");
}

HeaderBar::~HeaderBar()
{
}

void HeaderBar::layout()
{
    double w = geometry().width;
    double h = geometry().height;

    if (m_titleLabel) {
        m_titleLabel->setGeometry(ui::RectF(24, (h - 24) / 2, 120, 24));
    }

    if (m_searchField) {
        double fieldWidth = 260.0;
        double fieldHeight = 30.0;
        m_searchField->setGeometry(ui::RectF(w - fieldWidth - 140, (h - fieldHeight) / 2, fieldWidth, fieldHeight));
    }
}

void HeaderBar::paint(ui::Painter& painter)
{
    ui::RectF bounds = localRect();
    bool dark = true; // Dark mode default for photos

    painter.fillRect(bounds, dark ? ui::Color(0x1a, 0x1a, 0x1e) : ui::Color(0xff, 0xff, 0xff));
    painter.drawHorizontalLine(0, bounds.height - 1, bounds.width, dark ? ui::Color(0x2c, 0x2c, 0x32) : ui::Color(0xe0, 0xe0, 0xe4));

    // Slideshow button icon placeholder at top right
    ui::RectF slideRect(bounds.width - 110, (bounds.height - 28) / 2, 32, 28);
    painter.fillRoundedRect(slideRect, 6.0, ui::Color(0xff, 0xff, 0xff, 0x15));

    // Delete button icon placeholder at top right
    ui::RectF delRect(bounds.width - 64, (bounds.height - 28) / 2, 32, 28);
    painter.fillRoundedRect(delRect, 6.0, ui::Color(0xff, 0xff, 0xff, 0x15));
}

void HeaderBar::pointerPress(const ui::PointerEvent& event)
{
    ui::RectF bounds = localRect();
    ui::RectF slideRect(bounds.width - 110, (bounds.height - 28) / 2, 32, 28);
    ui::RectF delRect(bounds.width - 64, (bounds.height - 28) / 2, 32, 28);

    if (slideRect.contains(event.position)) {
        slideshowClicked.emit();
    } else if (delRect.contains(event.position)) {
        deleteClicked.emit();
    } else {
        if (window())
            window()->startMove();
    }
}

} // namespace cutefish::photos
