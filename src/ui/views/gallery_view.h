// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "backend/photo_store.h"
#include "base/signal.h"
#include "ui/label.h"
#include "ui/view.h"

#include <vector>

namespace cutefish::photos {

class GalleryView : public ui::View
{
public:
    GalleryView();
    ~GalleryView() override;

    base::Signal<PhotoItem> photoClicked;
    base::Signal<PhotoItem> photoDoubleClicked;

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerPress(const ui::PointerEvent& event) override;

private:
    void reloadItems();
    std::vector<PhotoItem> m_items;
    std::vector<ui::Label*> m_labels;
    size_t m_selectedIndex = 0;
};

} // namespace cutefish::photos
