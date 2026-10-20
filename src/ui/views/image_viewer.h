// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "backend/photo_store.h"
#include "base/signal.h"
#include "ui/label.h"
#include "ui/view.h"

namespace cutefish::photos {

class ImageViewer : public ui::View
{
public:
    ImageViewer(const PhotoItem& photo);
    ~ImageViewer() override;

    base::Signal<> closeRequested;
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerPress(const ui::PointerEvent& event) override;

private:
    PhotoItem m_photo;
    ui::Label* m_titleLabel = nullptr;
    ui::Label* m_backLabel = nullptr;
    double m_zoom = 1.0;
    double m_rotation = 0.0;
};

} // namespace cutefish::photos
