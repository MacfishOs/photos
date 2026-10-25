// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "backend/photo_store.h"
#include "base/signal.h"
#include "base/timer.h"
#include "ui/image_view.h"
#include "ui/label.h"
#include "ui/round_image_button.h"
#include "ui/slider.h"
#include "ui/view.h"
#include "ui/views/image_viewer.h"

#include <vector>

namespace cutefish::photos {

class SlideshowView : public ui::View
{
public:
    SlideshowView();
    ~SlideshowView() override;

    void start(const std::vector<PhotoItem>& photos, int startIndex = 0);
    void stop();

    base::Signal<> exitClicked;

    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;

private:
    void showCurrentPhoto();
    void nextPhoto();
    void prevPhoto();

    std::vector<PhotoItem> m_photos;
    int m_currentIndex = 0;
    base::Timer m_timer;

    ui::ImageView* m_imageView = nullptr;

    // Top Floating Header Bar
    ui::RoundImageButton* m_backBtn = nullptr;
    ui::Slider* m_volumeSlider = nullptr;
    ui::Label* m_headerTitleLabel = nullptr;
    ui::Label* m_headerSubLabel = nullptr;
    ui::RoundImageButton* m_shareBtn = nullptr;
    ui::RoundImageButton* m_favoriteBtn = nullptr;
    ui::Label* m_customizePill = nullptr;

    // Center Memory Overlay Title
    ui::Label* m_overlayTitleLabel = nullptr;
    ui::Label* m_overlaySubLabel = nullptr;

    // Bottom Glass Circular Buttons & Filmstrip
    ui::RoundImageButton* m_prevBtn = nullptr;
    ui::RoundImageButton* m_musicBtn = nullptr;
    ui::RoundImageButton* m_styleBtn = nullptr;
    FilmstripCarouselView* m_filmstrip = nullptr;
    ui::RoundImageButton* m_pauseBtn = nullptr;
    ui::RoundImageButton* m_gridBtn = nullptr;
};

} // namespace cutefish::photos
