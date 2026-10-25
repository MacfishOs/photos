// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/header_bar.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

namespace cutefish::photos {

static const char* kAppleFontFamily = "SF Pro Text, Adwaita Sans, Cantarell, Noto Sans, sans-serif";

// ============================================================================
// HeaderBar
// ============================================================================

HeaderBar::HeaderBar()
{
    m_backBtn = addChild<ui::RoundImageButton>("apple-chevron-left", 28);
    m_backBtn->setIconSize(16);
    m_backBtn->setVisible(false);
    m_backBtn->onClicked = [this]() { backClicked.emit(); };

    m_titleLabel = addChild<ui::Label>("Library");
    m_titleLabel->setFontFamily(kAppleFontFamily);
    m_titleLabel->setFontSize(15);
    m_titleLabel->setWeight(700);

    m_subtitleLabel = addChild<ui::Label>("All Photos");
    m_subtitleLabel->setFontFamily(kAppleFontFamily);
    m_subtitleLabel->setFontSize(11);
    m_subtitleLabel->setColor(ui::Color::rgb(0x8E8E93));

    m_infoBtn = addChild<ui::RoundImageButton>("apple-info", 28);
    m_infoBtn->setIconSize(16);
    m_infoBtn->onClicked = [this]() { infoPanelToggled.emit(); };

    m_themeBtn = addChild<ui::RoundImageButton>("apple-moon", 28);
    m_themeBtn->setIconSize(16);
    m_themeBtn->onClicked = [this]() { themeToggled.emit(); };

    m_searchField = addChild<ui::SearchField>();
    m_searchField->setPlaceholder("Search");
    m_searchField->onTextChanged = [this]() {
        searchChanged.emit(m_searchField->text());
    };
}

void HeaderBar::setBackButtonVisible(bool visible)
{
    m_backBtn->setVisible(visible);
    requestLayout();
}

void HeaderBar::setTitle(const std::string& title, const std::string& subtitle)
{
    m_titleLabel->setText(title);
    m_subtitleLabel->setText(subtitle);
    requestLayout();
    requestPaint();
}

ui::SizeF HeaderBar::sizeHint() const
{
    return {800, 52};
}

void HeaderBar::layout()
{
    double w = localRect().width;
    double h = localRect().height;
    if (w <= 0 || h <= 0) return;

    double x = 16.0;

    // Back button
    if (m_backBtn->isVisible()) {
        m_backBtn->setGeometry({x, (h - 28.0) / 2.0, 28.0, 28.0});
        x += 36.0;
    } else {
        m_backBtn->setGeometry({0, 0, 0, 0});
    }

    // Title and Subtitle
    m_titleLabel->setGeometry({x, 8.0, 220.0, 18.0});
    m_subtitleLabel->setGeometry({x, 26.0, 220.0, 16.0});

    // Right Action Cluster (Search, Info, Theme)
    double rightX = w - 16.0;

    double searchW = 160.0;
    rightX -= searchW;
    m_searchField->setGeometry({rightX, (h - 28.0) / 2.0, searchW, 28.0});
    rightX -= 12.0;

    m_themeBtn->setGeometry({rightX - 28.0, (h - 28.0) / 2.0, 28.0, 28.0});
    rightX -= 36.0;

    m_infoBtn->setGeometry({rightX - 28.0, (h - 28.0) / 2.0, 28.0, 28.0});
}

void HeaderBar::paint(ui::Painter& painter)
{
    bool dark = ui::Theme::current().darkMode;

    ui::Color bg = dark ? ui::Color::fromArgb(0xB025252A)
                        : ui::Color::fromArgb(0xB0F8F8FA);
    painter.fillRect(localRect(), bg);

    ui::Color border = dark ? ui::Color::fromArgb(0x28FFFFFF)
                            : ui::Color::fromArgb(0x18000000);
    painter.drawHorizontalLine(0, localRect().width, localRect().height - 1, border);
}

void HeaderBar::pointerPress(const ui::PointerEvent& event)
{
    (void)event;
    if (window()) {
        window()->startMove();
    }
}

} // namespace cutefish::photos
