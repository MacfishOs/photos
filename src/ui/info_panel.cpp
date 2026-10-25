// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/info_panel.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

namespace cutefish::photos {

InfoPanel::InfoPanel()
{
    m_headerLabel = addChild<ui::Label>("Info");
    m_headerLabel->setFontSize(14);
    m_headerLabel->setWeight(700);

    m_previewImage = addChild<ui::ImageView>();
    m_previewImage->setThumbnail(true);
    m_previewImage->setSourceSize({240, 160});
    m_previewImage->setCornerRadius(8.0);
    m_previewImage->setFillMode(ui::ImageView::FillMode::PreserveAspectCrop);

    m_titleLabel = addChild<ui::Label>("No item selected");
    m_titleLabel->setFontSize(13);
    m_titleLabel->setWeight(600);
    m_titleLabel->setElide(ui::Label::Elide::Right);

    auto createInfoRow = [this](const std::string& key) -> ui::Label* {
        auto keyLbl = addChild<ui::Label>(key);
        keyLbl->setFontSize(11);
        keyLbl->setColor(ui::Color::rgb(0x888888));

        auto valLbl = addChild<ui::Label>("-");
        valLbl->setFontSize(11);
        valLbl->setElide(ui::Label::Elide::Right);

        m_infoRows.push_back({keyLbl, valLbl});
        return valLbl;
    };

    m_dateVal = createInfoRow("Date");
    m_dimensionsVal = createInfoRow("Dimensions");
    m_sizeVal = createInfoRow("Size");
    m_fileTypeVal = createInfoRow("Format");
    m_pathVal = createInfoRow("Location");
}

void InfoPanel::setPhoto(const PhotoItem* photo)
{
    if (!photo) {
        m_titleLabel->setText("No item selected");
        m_previewImage->setSource("");
        m_dateVal->setText("-");
        m_dimensionsVal->setText("-");
        m_sizeVal->setText("-");
        m_fileTypeVal->setText("-");
        m_pathVal->setText("-");
    } else {
        m_titleLabel->setText(photo->title);
        m_previewImage->setSource(photo->path);
        m_dateVal->setText(photo->formattedDate());
        m_dimensionsVal->setText(photo->formattedDimensions());
        m_sizeVal->setText(photo->formattedSize());

        auto pos = photo->path.rfind('.');
        std::string ext = (pos != std::string::npos) ? photo->path.substr(pos + 1) : "IMAGE";
        for (char& c : ext) c = toupper(c);
        m_fileTypeVal->setText(ext);

        m_pathVal->setText(photo->path);
    }

    requestLayout();
    requestPaint();
}

ui::SizeF InfoPanel::sizeHint() const
{
    return {260, 400};
}

void InfoPanel::layout()
{
    double w = localRect().width;
    double h = localRect().height;
    double padding = 16.0;

    if (w <= 0 || h <= 0) return;

    if (window()) {
        window()->setBackgroundBlur(ui::RectF(window()->size().width - w, 0, w, h), 20.0);
    }

    m_headerLabel->setGeometry({padding, 16, w - padding * 2, 20});

    double imgSize = w - padding * 2.0;
    m_previewImage->setGeometry({padding, 48, imgSize, imgSize * 0.65});

    double y = 48 + imgSize * 0.65 + 16.0;
    m_titleLabel->setGeometry({padding, y, w - padding * 2.0, 20});
    y += 28.0;

    for (auto& row : m_infoRows) {
        row.keyLabel->setGeometry({padding, y, 80, 18});
        row.valLabel->setGeometry({padding + 85, y, w - padding * 2.0 - 85, 18});
        y += 22.0;
    }
}

void InfoPanel::paint(ui::Painter& painter)
{
    bool dark = ui::Theme::current().darkMode;

    ui::Color bg = dark ? ui::Color::fromArgb(0x8018181C)
                        : ui::Color::fromArgb(0x80F4F4F6);
    painter.fillRect(localRect(), bg);

    // Left border divider line
    ui::Color border = dark ? ui::Color::fromArgb(0x28FFFFFF)
                            : ui::Color::fromArgb(0x18000000);
    painter.drawVerticalLine(0, 0, localRect().height, border);
}

} // namespace cutefish::photos
