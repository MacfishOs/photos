// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "backend/photo_store.h"
#include "base/signal.h"
#include "ui/image_view.h"
#include "ui/label.h"
#include "ui/scroll_view.h"
#include "ui/view.h"

#include <vector>

namespace cutefish::photos {

class ClickableImageView : public ui::ImageView
{
public:
    explicit ClickableImageView(std::string path = {}) : ui::ImageView(std::move(path)) {}

    bool acceptsPointer() const override { return true; }

    std::function<void()> onClicked;

protected:
    void pointerPress(const ui::PointerEvent& event) override
    {
        (void)event;
        if (onClicked) onClicked();
    }
};

class AlbumCardView : public ui::View
{
public:
    AlbumCardView(const AlbumItem& album);
    ~AlbumCardView() override = default;

    const AlbumItem& album() const { return m_album; }

    std::function<void()> onClicked;
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerPress(const ui::PointerEvent& event) override;

private:
    AlbumItem m_album;
    ClickableImageView* m_imageView = nullptr;
    ui::Label* m_titleLabel = nullptr;
};

class GalleryCardView : public ui::View
{
public:
    GalleryCardView(const PhotoItem& photo, double cardWidth);
    ~GalleryCardView() override = default;

    void setCardWidth(double width);
    void setSelected(bool selected);
    bool isSelected() const { return m_selected; }
    void setCardVisible(bool visible);

    const PhotoItem& item() const { return m_item; }

    std::function<void()> onClicked;

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const ui::PointerEvent& event) override;

private:
    PhotoItem m_item;
    double m_cardWidth;
    bool m_selected = false;
    bool m_hovered = false;
    bool m_loaded = false;

    ClickableImageView* m_imageView = nullptr;
    ui::ImageView* m_heartIcon = nullptr;
};

class GalleryContentView : public ui::View
{
public:
    GalleryContentView();
    ~GalleryContentView() override = default;

    void refresh();
    void setThumbnailSize(double size);
    void updateVisibleViewport(ui::PointF scrollPos, double viewportHeight);

    const PhotoItem* selectedPhoto() const;

    base::Signal<PhotoItem> photoClicked;
    base::Signal<PhotoItem> photoDoubleClicked;
    base::Signal<AlbumItem> albumClicked;

    ui::SizeF sizeHint() const override { return {0, m_totalHeight}; }
    double heightForWidth(double) const override { return m_totalHeight; }
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void pointerPress(const ui::PointerEvent& event) override;

private:
    void rebuildGrid();
    void updateCardVisibility();

    std::vector<PhotoItem> m_photos;
    std::vector<AlbumItem> m_albums;
    std::vector<GalleryCardView*> m_photoCards;
    std::vector<AlbumCardView*> m_albumCards;

    double m_cardSize = 220.0;
    double m_totalHeight = 400.0;
    int m_selectedIndex = -1;
    ui::PointF m_scrollPos{0, 0};
    double m_viewportHeight = 800.0;
};

class GalleryView : public ui::View
{
public:
    GalleryView();
    ~GalleryView() override = default;

    void setThumbnailSize(double size);

    const PhotoItem* selectedPhoto() const { return m_content ? m_content->selectedPhoto() : nullptr; }

    base::Signal<PhotoItem> photoClicked;
    base::Signal<PhotoItem> photoDoubleClicked;
    base::Signal<AlbumItem> albumClicked;

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;

private:
    ui::ScrollView* m_scrollView = nullptr;
    GalleryContentView* m_content = nullptr;
    base::Subscription m_storeSub;
    std::vector<base::Subscription> m_subs;
};

} // namespace cutefish::photos
