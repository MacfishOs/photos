// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "backend/photo_store.h"
#include "base/signal.h"
#include "ui/icon.h"
#include "ui/image_view.h"
#include "ui/label.h"
#include "ui/round_image_button.h"
#include "ui/view.h"
#include "ui/window_controls.h"

#include <string>
#include <vector>

namespace cutefish::photos {

class SidebarView : public ui::View
{
public:
    SidebarView();
    ~SidebarView() override = default;

    base::Signal<NavCategory> categorySelected;
    base::Signal<> sidebarToggleClicked;

    void updateCounts();
    void selectCategory(NavCategory category);

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerPress(const ui::PointerEvent& event) override;

private:
    struct SidebarEntry
    {
        bool isHeader = false;
        std::string sectionTitle;

        std::string icon;
        std::string title;
        NavCategory category = NavCategory::Library;
        std::string albumCoverPath;
        std::string folderPath;
        int indentLevel = 0;
        bool isLocked = false;

        ui::Label* headerLabel = nullptr;
        ui::Label* label = nullptr;
        ui::ThemeIcon* themeIcon = nullptr;
        ui::ImageView* coverThumb = nullptr;
        ui::ThemeIcon* lockIcon = nullptr;
    };

    WindowControls* m_windowControls = nullptr;

    std::vector<SidebarEntry> m_entries;
    size_t m_selectedIndex = 0;
    base::Subscription m_storeSub;
};

} // namespace cutefish::photos
