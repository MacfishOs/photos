// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "base/signal.h"
#include "ui/label.h"
#include "ui/search_field.h"
#include "ui/view.h"

namespace cutefish::photos {

class HeaderBar : public ui::View
{
public:
    HeaderBar();
    ~HeaderBar() override;

    base::Signal<std::string> searchChanged;
    base::Signal<> slideshowClicked;
    base::Signal<> deleteClicked;

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerPress(const ui::PointerEvent& event) override;

private:
    ui::SearchField* m_searchField = nullptr;
    ui::Label* m_titleLabel = nullptr;
};

} // namespace cutefish::photos
