// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "backend/photo_store.h"
#include "base/signal.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/round_image_button.h"
#include "ui/search_field.h"
#include "ui/slider.h"
#include "ui/view.h"
#include "ui/window_controls.h"

namespace cutefish::photos {

class HeaderBar : public ui::View
{
public:
    HeaderBar();
    ~HeaderBar() override = default;

    void setTitle(const std::string& title, const std::string& subtitle = {});
    void setBackButtonVisible(bool visible);

    base::Signal<std::string> searchChanged;
    base::Signal<> themeToggled;
    base::Signal<> wallpaperClicked;
    base::Signal<> slideshowClicked;
    base::Signal<> infoPanelToggled;
    base::Signal<> shareClicked;
    base::Signal<> favoriteClicked;
    base::Signal<> backClicked;

    ui::SizeF sizeHint() const override;
    bool acceptsPointer() const override { return true; }
    void pointerPress(const ui::PointerEvent& event) override;

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;

private:
    ui::RoundImageButton* m_backBtn = nullptr;

    ui::Label* m_titleLabel = nullptr;
    ui::Label* m_subtitleLabel = nullptr;

    ui::RoundImageButton* m_personFilterBtn = nullptr;
    ui::RoundImageButton* m_viewOptionsBtn = nullptr;
    ui::RoundImageButton* m_infoBtn = nullptr;
    ui::RoundImageButton* m_shareBtn = nullptr;
    ui::RoundImageButton* m_favoriteBtn = nullptr;
    ui::RoundImageButton* m_themeBtn = nullptr;

    ui::SearchField* m_searchField = nullptr;
};

} // namespace cutefish::photos
