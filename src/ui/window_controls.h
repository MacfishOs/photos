// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "ui/view.h"

namespace cutefish::photos {

class WindowControls : public ui::View
{
public:
    WindowControls();
    ~WindowControls() override = default;

    ui::SizeF sizeHint() const override;
    bool acceptsPointer() const override { return true; }

protected:
    void paint(ui::Painter& painter) override;
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const ui::PointerEvent& event) override;
    void pointerRelease(const ui::PointerEvent& event) override;

private:
    enum class ButtonType {
        None,
        Close,
        Minimize,
        Maximize
    };

    ButtonType buttonAt(ui::PointF point) const;

    bool m_hovered = false;
    ButtonType m_pressedButton = ButtonType::None;
};

} // namespace cutefish::photos
