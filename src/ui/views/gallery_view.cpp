// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/views/gallery_view.h"
#include "ui/painter.h"

namespace cutefish::photos {

GalleryView::GalleryView()
{
    reloadItems();
    PhotoStore::instance().photosChanged.connect([this]() {
        reloadItems();
    });
}

GalleryView::~GalleryView()
{
}

void GalleryView::reloadItems()
{
    for (auto* lbl : m_labels)
        takeChild(lbl);
    m_labels.clear();

    m_items = PhotoStore::instance().photos();

    for (const auto& item : m_items) {
        auto* lbl = addChild<ui::Label>(item.title);
        lbl->setFontSize(12.0);
        lbl->setWeight(600);
        lbl->setColor(ui::Color(0xf5, 0xf5, 0xf7));
        m_labels.push_back(lbl);
    }

    requestLayout();
    requestPaint();
}

void GalleryView::layout()
{
    double w = geometry().width;
    double cardWidth = 200.0;
    double cardHeight = 160.0;
    double spacing = 20.0;
    double margin = 24.0;

    double x = margin;
    double y = margin;

    for (size_t i = 0; i < m_items.size(); ++i) {
        if (x + cardWidth > w - margin) {
            x = margin;
            y += cardHeight + spacing + 24.0;
        }

        if (i < m_labels.size()) {
            m_labels[i]->setGeometry(ui::RectF(x, y + cardHeight + 8.0, cardWidth, 20.0));
        }

        x += cardWidth + spacing;
    }
}

void GalleryView::paint(ui::Painter& painter)
{
    ui::RectF bounds = localRect();
    bool dark = true;

    painter.fillRect(bounds, dark ? ui::Color(0x12, 0x12, 0x16) : ui::Color(0xf8, 0xf8, 0xfc));

    double cardWidth = 200.0;
    double cardHeight = 160.0;
    double spacing = 20.0;
    double margin = 24.0;

    double x = margin;
    double y = margin;

    for (size_t i = 0; i < m_items.size(); ++i) {
        if (x + cardWidth > bounds.width - margin) {
            x = margin;
            y += cardHeight + spacing + 24.0;
        }

        ui::RectF cardRect(x, y, cardWidth, cardHeight);
        painter.fillRoundedRect(cardRect, 12.0, dark ? ui::Color(0x22, 0x22, 0x28) : ui::Color(0xff, 0xff, 0xff));

        if (i == m_selectedIndex) {
            painter.drawRoundedRect(cardRect, ui::RoundedShape(12.0), ui::Color(0, 0, 0, 0), 2.0, ui::Color(0x0a, 0x84, 0xff));
        }

        x += cardWidth + spacing;
    }
}

void GalleryView::pointerPress(const ui::PointerEvent& event)
{
    double cardWidth = 200.0;
    double cardHeight = 160.0;
    double spacing = 20.0;
    double margin = 24.0;

    double x = margin;
    double y = margin;

    for (size_t i = 0; i < m_items.size(); ++i) {
        if (x + cardWidth > geometry().width - margin) {
            x = margin;
            y += cardHeight + spacing + 24.0;
        }

        ui::RectF cardRect(x, y, cardWidth, cardHeight + 30.0);
        if (cardRect.contains(event.position)) {
            m_selectedIndex = i;
            photoClicked.emit(m_items[i]);
            requestPaint();
            break;
        }

        x += cardWidth + spacing;
    }
}

} // namespace cutefish::photos
