// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/application.h"
#include "ui/apple_icons.h"
#include "ui/main_window.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
    cutefish::ui::Application app(argc, argv);
    app.setApplicationId("cutefish-photos");

    cutefish::photos::registerAppleIcons();

    cutefish::photos::MainWindow window;

    if (argc > 1) {
        std::string arg = argv[1];
        if (fs::exists(arg)) {
            fs::path absPath = fs::absolute(arg);
            if (fs::is_regular_file(absPath)) {
                std::string parentDir = absPath.parent_path().string();
                window.openDirectory(parentDir, absPath.string());
            } else if (fs::is_directory(absPath)) {
                window.openDirectory(absPath.string());
            }
        }
    }

    window.show();

    return app.run();
}
