// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/sidebar_view.h"
#include "ui/painter.h"

namespace cutefish::photos {

static bool isDarkMode()
{
    return true;
}

SidebarView::SidebarView()
{
    std::vector<std::pair<std::string, NavCategory>> cats = {
        {"Photos", NavCategory::Photos},
        {"Favorites", NavCategory::Favorites},
        {"Desktop", NavCategory::Desktop},
        {"Documents", NavCategory::Documents},
        {"Downloads", NavCategory::Downloads},
        {"Screenshots", NavCategory::Screenshots},
        {"Trash", NavCategory::Trash}
    };

    for (const auto& [title, cat] : cats) {
        auto* lbl = addChild<ui::Label>(title);
        lbl->setFontSize(13.0);
        lbl->setWeight(500);
        lbl->setColor(ui::Color(0xf5, 0xf5, 0xf7));
        m_items.push_back({title, cat, lbl});
    }
}

SidebarView::~SidebarView()
{
}

void SidebarView::layout()
{
    double w = geometry().width;
    double y = 70.0;

    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].label) {
            m_items[i].label->setGeometry(ui::RectF(24, y, w - 48, 32));
        }
        y += 40.0;
    }
}

void SidebarView::paint(ui::Painter& painter)
{
    ui::RectF bounds = localRect();
    bool dark = isDarkMode();

    // Sidebar background blur & fill
    painter.fillRect(bounds, dark ? ui::Color(0x16, 0x16, 0x1a, 0xf0) : ui::Color(0xf2, 0xf2, 0xf7, 0xf0));
    painter.drawVerticalLine(bounds.width - 1, 0, bounds.height, dark ? ui::Color(0x2c, 0x2c, 0x32) : ui::Color(0xe0, 0xe0, 0xe4));

    // Paint selected item indicator highlight pill
    double y = 70.0 + (m_selectedIndex * 40.0);
    ui::RectF highlightRect(12, y - 2, bounds.width - 24, 32);
    painter.fillRoundedRect(highlightRect, 8.0, dark ? ui::Color(0xff, 0xff, 0xff, 0x18) : ui::Color(0x00, 0x00, 0x00, 0x08));
}

void SidebarView::pointerPress(const ui::PointerEvent& event)
{
    double y = 70.0;
    for (size_t i = 0; i < m_items.size(); ++i) {
        ui::RectF itemRect(12, y - 2, geometry().width - 24, 32);
        if (itemRect.contains(event.position)) {
            m_selectedIndex = i;
            categorySelected.emit(m_items[i].category);
            requestPaint();
            break;
        }
        y += 40.0;
    }
}

} // namespace cutefish::photos
