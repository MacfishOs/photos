// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/window_controls.h"
#include <cmath>
#include "include/core/SkImage.h"
#include "ui/application.h"
#include "ui/icon_loader.h"
#include "ui/painter.h"
#include "ui/window.h"

namespace cutefish::photos {

WindowControls::WindowControls()
{
}

ui::SizeF WindowControls::sizeHint() const
{
    return {60, 20};
}

WindowControls::ButtonType WindowControls::buttonAt(ui::PointF point) const
{
    double h = localRect().height;
    double cy = h > 0 ? h / 2.0 : 10.0;
    double r = 8.0;

    if (point.y < cy - r || point.y > cy + r) {
        return ButtonType::None;
    }

    if (point.x >= 0.0 && point.x <= 20.0) {
        return ButtonType::Close;
    } else if (point.x > 20.0 && point.x <= 40.0) {
        return ButtonType::Minimize;
    } else if (point.x > 40.0 && point.x <= 60.0) {
        return ButtonType::Maximize;
    }

    return ButtonType::None;
}

void WindowControls::pointerEnter()
{
    m_hovered = true;
    requestPaint();
}

void WindowControls::pointerLeave()
{
    m_hovered = false;
    requestPaint();
}

void WindowControls::pointerPress(const ui::PointerEvent& event)
{
    ButtonType btn = buttonAt(event.position);
    if (btn == ButtonType::None) {
        return;
    }
    m_pressedButton = btn;
    requestPaint();
}

void WindowControls::pointerRelease(const ui::PointerEvent& event)
{
    ButtonType btn = buttonAt(event.position);
    ButtonType pressed = m_pressedButton;
    m_pressedButton = ButtonType::None;
    requestPaint();

    if (btn != pressed || btn == ButtonType::None || !window()) {
        return;
    }

    if (btn == ButtonType::Close) {
        if (window()->onCloseRequested) {
            window()->onCloseRequested();
        } else if (ui::Application::instance()) {
            ui::Application::instance()->quit();
        }
    } else if (btn == ButtonType::Minimize) {
        window()->minimize();
    } else if (btn == ButtonType::Maximize) {
        bool max = window()->states().maximized;
        window()->setMaximized(!max);
    }
}

void WindowControls::paint(ui::Painter& painter)
{
    const ui::Window* win = window();
    const bool active = win && win->isActive();
    const bool anyPressed = m_pressedButton != ButtonType::None;
    const double opacity = (active || m_hovered || anyPressed) ? 1.0 : 0.35;

    double h = localRect().height;
    double cy = std::round(h > 0 ? h / 2.0 : 10.0);
    const double radius = 7.0; // logical 14px diameter
    const double size = radius * 2.0;

    ui::Color closeColor = ui::Color::rgb(0xFF5F56);
    ui::Color minColor = ui::Color::rgb(0xFFBD2E);
    ui::Color maxColor = ui::Color::rgb(0x27C93F);

    if (m_pressedButton == ButtonType::Close) closeColor = closeColor.darker(1.2);
    if (m_pressedButton == ButtonType::Minimize) minColor = minColor.darker(1.2);
    if (m_pressedButton == ButtonType::Maximize) maxColor = maxColor.darker(1.2);

    // Close dot
    ui::Rect closeRect = painter.snappedRect({std::round(10.0 - radius), std::round(cy - radius), size, size});
    painter.fillDeviceRoundedRect({closeRect.x, closeRect.y, closeRect.width, closeRect.height},
                                 closeRect.width / 2.0, closeColor.withAlpha(opacity));

    // Minimize dot
    ui::Rect minRect = painter.snappedRect({std::round(30.0 - radius), std::round(cy - radius), size, size});
    painter.fillDeviceRoundedRect({minRect.x, minRect.y, minRect.width, minRect.height},
                                 minRect.width / 2.0, minColor.withAlpha(opacity));

    // Maximize dot
    ui::Rect maxRect = painter.snappedRect({std::round(50.0 - radius), std::round(cy - radius), size, size});
    painter.fillDeviceRoundedRect({maxRect.x, maxRect.y, maxRect.width, maxRect.height},
                                 maxRect.width / 2.0, maxColor.withAlpha(opacity));

    // Hover or pressed glyphs
    if (m_hovered || anyPressed) {
        const ui::Color closeInk = ui::Color::rgb(0x4B0000);
        sk_sp<SkImage> closeImg = ui::IconLoader::instance().glyph("window-close", size, closeInk);
        if (closeImg) {
            painter.drawImage(closeImg.get(), closeRect.x, closeRect.y);
        }

        const ui::Color minInk = ui::Color::rgb(0x965000);
        sk_sp<SkImage> minImg = ui::IconLoader::instance().glyph("window-minimize", size, minInk);
        if (minImg) {
            painter.drawImage(minImg.get(), minRect.x, minRect.y);
        }

        const std::string maxName = (win && win->states().maximized) ? "window-restore" : "window-maximize";
        const ui::Color maxInk = ui::Color::rgb(0x005F00);
        sk_sp<SkImage> maxImg = ui::IconLoader::instance().glyph(maxName, size, maxInk);
        if (maxImg) {
            painter.drawImage(maxImg.get(), maxRect.x, maxRect.y);
        }
    }
}

} // namespace cutefish::photos
