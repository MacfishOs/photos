// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/sidebar_view.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

namespace cutefish::photos {

static const char* kAppleFontFamily = "SF Pro Text, Adwaita Sans, Cantarell, Noto Sans, sans-serif";

SidebarView::SidebarView()
{
    // Traffic Lights (Red, Yellow, Green) at top left of window sidebar
    m_windowControls = addChild<WindowControls>();

    auto addSectionHeader = [this](const std::string& title) {
        SidebarEntry header;
        header.isHeader = true;
        header.sectionTitle = title;
        header.headerLabel = addChild<ui::Label>(title);
        header.headerLabel->setFontFamily(kAppleFontFamily);
        header.headerLabel->setFontSize(11);
        header.headerLabel->setWeight(600);
        header.headerLabel->setColor(ui::Color::rgb(0x8E8E93));
        m_entries.push_back(header);
    };

    auto addItem = [this](const std::string& title, const std::string& iconName, NavCategory category, int indent = 0) {
        SidebarEntry item;
        item.title = title;
        item.icon = iconName;
        item.category = category;
        item.indentLevel = indent;

        item.themeIcon = addChild<ui::ThemeIcon>(iconName, 16);

        item.label = addChild<ui::Label>(title);
        item.label->setFontFamily(kAppleFontFamily);
        item.label->setFontSize(13);
        item.label->setWeight(500);

        m_entries.push_back(item);
    };

    const char* home = std::getenv("HOME");
    std::string homeDir = home ? home : "/home/jatin";

    // 1. Library Categories
    addSectionHeader("Library");
    addItem("All Photos", "folder-pictures-symbolic", NavCategory::Library);
    addItem("Favorites", "emblem-favorite-symbolic", NavCategory::Favorites);
    addItem("Recents", "folder-download-symbolic", NavCategory::RecentlySaved);

    // 2. Local Folders & Subfolders
    addSectionHeader("Folders");
    addItem("Pictures", "folder-pictures-symbolic", NavCategory::FolderPictures);
    addItem("Downloads", "folder-download-symbolic", NavCategory::FolderDownloads);

    // Subfolders discovered during PhotoStore scan
    auto folders = PhotoStore::instance().folders();
    for (const auto& folder : folders) {
        // Skip base root folders that are already listed above
        if (folder.path == homeDir + "/Pictures" ||
            folder.path == homeDir + "/Downloads" ||
            folder.path == homeDir + "/Desktop" ||
            folder.path == homeDir + "/Documents" ||
            folder.path == homeDir + "/Photos") {
            continue;
        }

        SidebarEntry item;
        item.title = folder.name + " (" + std::to_string(folder.photoCount) + ")";
        item.icon = "folder-symbolic";
        item.category = NavCategory::CustomFolder;
        item.folderPath = folder.path;
        item.indentLevel = 0; // Flat list, no subfolder indentation

        item.themeIcon = addChild<ui::ThemeIcon>("folder-symbolic", 16);

        item.label = addChild<ui::Label>(item.title);
        item.label->setFontFamily(kAppleFontFamily);
        item.label->setFontSize(13);
        item.label->setWeight(500);

        m_entries.push_back(item);
    }

    m_storeSub = PhotoStore::instance().photosChanged.connect([this]() {
        updateCounts();
    });

    updateCounts();
}

void SidebarView::updateCounts()
{
    requestPaint();
}

void SidebarView::selectCategory(NavCategory category)
{
    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (!m_entries[i].isHeader && m_entries[i].category == category) {
            m_selectedIndex = i;
            break;
        }
    }
    requestPaint();
}

void SidebarView::layout()
{
    double w = localRect().width;
    double h = localRect().height;

    if (w <= 0 || h <= 0) return;

    // Apply Cutefish window background blur on the sidebar region
    if (window()) {
        window()->setBackgroundBlur(ui::RectF(0, 0, w, h), 16.0);
    }

    m_windowControls->setGeometry({12, 14, 60, 20});

    double y = 48.0;
    double itemH = 32.0;

    for (size_t i = 0; i < m_entries.size(); ++i) {
        auto& entry = m_entries[i];
        if (entry.isHeader) {
            y += 8.0;
            if (entry.headerLabel) {
                entry.headerLabel->setGeometry({16, y, w - 32, 16});
            }
            y += 20.0;
        } else {
            double indentX = entry.indentLevel * 14.0;

            if (entry.coverThumb) {
                entry.coverThumb->setGeometry({18.0 + indentX, y + 6, 20, 20});
            } else if (entry.themeIcon) {
                entry.themeIcon->setGeometry({18.0 + indentX, y + 8, 16, 16});
            }

            if (entry.label) {
                entry.label->setGeometry({44.0 + indentX, y + 6, w - 74 - indentX, 20});
            }

            if (entry.lockIcon) {
                entry.lockIcon->setGeometry({w - 28, y + 9, 14, 14});
            }

            y += itemH + 2.0;
        }
    }
}

void SidebarView::pointerPress(const ui::PointerEvent& event)
{
    double y = 48.0;
    double itemH = 32.0;

    for (size_t i = 0; i < m_entries.size(); ++i) {
        auto& entry = m_entries[i];
        if (entry.isHeader) {
            y += 28.0;
        } else {
            ui::RectF rect{10.0, y, localRect().width - 20.0, itemH};
            if (rect.contains(event.position)) {
                m_selectedIndex = i;
                categorySelected.emit(entry.category);
                if (entry.category == NavCategory::CustomFolder && !entry.folderPath.empty()) {
                    PhotoStore::instance().filterByFolderPath(entry.folderPath);
                } else {
                    PhotoStore::instance().setCategory(entry.category);
                }
                requestPaint();
                break;
            }
            y += itemH + 2.0;
        }
    }
}

void SidebarView::paint(ui::Painter& painter)
{
    bool dark = ui::Theme::current().darkMode;

    // Cutefish style translucent acrylic background with window blur
    ui::Color bg = dark ? ui::Color::fromArgb(0x8018181C)
                        : ui::Color::fromArgb(0x80F4F4F6);
    painter.fillRect(localRect(), bg);

    double w = localRect().width;
    double y = 48.0;
    double itemH = 32.0;

    for (size_t i = 0; i < m_entries.size(); ++i) {
        auto& entry = m_entries[i];
        if (entry.isHeader) {
            y += 28.0;
        } else {
            bool isSelected = (i == m_selectedIndex);
            ui::RectF capsuleRect{10.0, y, w - 20.0, itemH};

            if (isSelected) {
                ui::Color selBg = ui::Color::rgb(0x007AFF);
                painter.fillRoundedRect(capsuleRect, 8.0, selBg);
            }

            if (entry.label) {
                if (isSelected) {
                    entry.label->setColor(ui::Color::rgb(0xFFFFFF));
                } else {
                    entry.label->setColor(dark ? ui::Color::rgb(0xF0F0F0) : ui::Color::rgb(0x1C1C1E));
                }
            }

            if (entry.themeIcon) {
                if (isSelected) {
                    entry.themeIcon->setColor(ui::Color::rgb(0xFFFFFF));
                } else {
                    entry.themeIcon->setColor(dark ? ui::Color::rgb(0x0A84FF) : ui::Color::rgb(0x007AFF));
                }
            }

            y += itemH + 2.0;
        }
    }

    ui::Color border = dark ? ui::Color::fromArgb(0x28FFFFFF)
                            : ui::Color::fromArgb(0x18000000);
    painter.drawVerticalLine(w - 1, 0, localRect().height, border);
}

} // namespace cutefish::photos
