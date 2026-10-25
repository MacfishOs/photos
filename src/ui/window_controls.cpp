// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/window_controls.h"
#include "ui/application.h"
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
    if (btn == ButtonType::None || !window()) {
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
    double h = localRect().height;
    double cy = h > 0 ? h / 2.0 : 10.0;
    double r = 6.0;

    // Red Close
    ui::RectF closeRect{10.0 - r, cy - r, r * 2.0, r * 2.0};
    ui::Color closeColor = ui::Color::rgb(0xFF5F56);
    painter.fillEllipse(closeRect, closeColor);

    // Yellow Minimize
    ui::RectF minRect{30.0 - r, cy - r, r * 2.0, r * 2.0};
    ui::Color minColor = ui::Color::rgb(0xFFBD2E);
    painter.fillEllipse(minRect, minColor);

    // Green Maximize
    ui::RectF maxRect{50.0 - r, cy - r, r * 2.0, r * 2.0};
    ui::Color maxColor = ui::Color::rgb(0x27C93F);
    painter.fillEllipse(maxRect, maxColor);

    // Hover glyphs
    if (m_hovered) {
        ui::Color glyphColor = ui::Color::fromArgb(0x99000000);

        // Close x
        painter.drawLine({10.0 - 2.5, cy - 2.5}, {10.0 + 2.5, cy + 2.5}, glyphColor, 1.2);
        painter.drawLine({10.0 + 2.5, cy - 2.5}, {10.0 - 2.5, cy + 2.5}, glyphColor, 1.2);

        // Minimize -
        painter.drawLine({30.0 - 3.0, cy}, {30.0 + 3.0, cy}, glyphColor, 1.2);

        // Maximize +
        painter.drawLine({50.0 - 2.5, cy - 2.5}, {50.0 + 2.5, cy + 2.5}, glyphColor, 1.2);
        painter.drawLine({50.0 + 2.5, cy - 2.5}, {50.0 - 2.5, cy + 2.5}, glyphColor, 1.2);
    }
}

} // namespace cutefish::photos
