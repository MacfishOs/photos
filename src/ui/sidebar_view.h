// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "backend/photo_store.h"
#include "base/signal.h"
#include "ui/label.h"
#include "ui/view.h"

#include <vector>

namespace cutefish::photos {

class SidebarView : public ui::View
{
public:
    SidebarView();
    ~SidebarView() override;

    base::Signal<NavCategory> categorySelected;
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerPress(const ui::PointerEvent& event) override;

private:
    struct NavItem
    {
        std::string title;
        NavCategory category;
        ui::Label* label = nullptr;
    };

    std::vector<NavItem> m_items;
    size_t m_selectedIndex = 0;
};

} // namespace cutefish::photos
