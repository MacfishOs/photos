// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "backend/photo_store.h"
#include "base/signal.h"
#include "include/core/SkImage.h"
#include "ui/event.h"
#include "ui/image_decoder.h"
#include "ui/image_view.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/round_image_button.h"
#include "ui/view.h"
#include "ui/window_controls.h"

#include <vector>

namespace cutefish::photos {

class FilmstripCarouselView : public ui::View
{
public:
    FilmstripCarouselView();
    ~FilmstripCarouselView() override = default;

    void setPhotos(const std::vector<PhotoItem>& photos);
    void setSelectedIndex(int index);

    base::Signal<int> photoSelected;

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerPress(const ui::PointerEvent& event) override;

private:
    void rebuildThumbnails();
    double scrollOffset() const;

    std::vector<PhotoItem> m_photos;
    int m_selectedIndex = -1;
    std::vector<ui::ImageView*> m_thumbs;
};

class ImageViewerHeaderBar : public ui::View
{
public:
    ImageViewerHeaderBar();
    ~ImageViewerHeaderBar() override = default;

    void setTitleInfo(const std::string& title, const std::string& subtitle);

    bool acceptsPointer() const override { return true; }

    base::Signal<> backClicked;
    base::Signal<> favoriteClicked;
    base::Signal<> rotateLeftClicked;
    base::Signal<> rotateRightClicked;
    base::Signal<> zoomInClicked;
    base::Signal<> zoomOutClicked;
    base::Signal<> infoClicked;
    base::Signal<> wallpaperClicked;
    base::Signal<> shareClicked;
    base::Signal<> trashClicked;

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerPress(const ui::PointerEvent& event) override;

private:
    ui::Label* m_titleLabel = nullptr;
    ui::Label* m_subtitleLabel = nullptr;

    WindowControls* m_windowControls = nullptr;
    ui::RoundImageButton* m_backBtn = nullptr;
    ui::RoundImageButton* m_favoriteBtn = nullptr;
    ui::RoundImageButton* m_infoBtn = nullptr;
    ui::RoundImageButton* m_shareBtn = nullptr;
    ui::RoundImageButton* m_trashBtn = nullptr;
};

class ImageViewer : public ui::View
{
public:
    ImageViewer();
    ~ImageViewer() override = default;

    void setPhoto(const PhotoItem& photo, int index = 0, int totalCount = 1);
    int currentIndex() const { return static_cast<int>(m_currentIndex); }
    const PhotoItem& currentPhoto() const { return m_photo; }

    base::Signal<> closeRequested;
    base::Signal<> prevClicked;
    base::Signal<> nextClicked;
    base::Signal<> infoPanelToggled;
    base::Signal<> wallpaperSet;
    base::Signal<> shareClicked;

    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerPress(const ui::PointerEvent& event) override;
    bool keyPress(const ui::KeyEvent& event) override;

private:
    void zoomIn();
    void zoomOut();
    void rotateLeft();
    void rotateRight();
    void reloadDecodedImage();

    PhotoItem m_photo;
    std::string m_currentPath;
    size_t m_currentIndex = 0;
    size_t m_totalCount = 1;

    double m_zoomLevel = 1.0;
    int m_rotationAngle = 0;

    sk_sp<SkImage> m_decodedImage;

    ImageViewerHeaderBar* m_headerBar = nullptr;
    ui::RoundImageButton* m_prevBtn = nullptr;
    ui::RoundImageButton* m_nextBtn = nullptr;
    FilmstripCarouselView* m_filmstrip = nullptr;

    std::vector<base::Subscription> m_subs;
};

} // namespace cutefish::photos
