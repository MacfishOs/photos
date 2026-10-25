// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "backend/photo_store.h"
#include "base/signal.h"
#include "ui/image_view.h"
#include "ui/label.h"
#include "ui/view.h"

#include <vector>

namespace cutefish::photos {

class InfoPanel : public ui::View
{
public:
    InfoPanel();
    ~InfoPanel() override = default;

    void setPhoto(const PhotoItem* photo);

    ui::SizeF sizeHint() const override;
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;

private:
    struct InfoRow
    {
        ui::Label* keyLabel = nullptr;
        ui::Label* valLabel = nullptr;
    };

    ui::Label* m_headerLabel = nullptr;
    ui::ImageView* m_previewImage = nullptr;
    ui::Label* m_titleLabel = nullptr;

    ui::Label* m_dateVal = nullptr;
    ui::Label* m_dimensionsVal = nullptr;
    ui::Label* m_sizeVal = nullptr;
    ui::Label* m_fileTypeVal = nullptr;
    ui::Label* m_pathVal = nullptr;

    std::vector<InfoRow> m_infoRows;
};

} // namespace cutefish::photos
