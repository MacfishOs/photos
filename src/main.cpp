// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/application.h"
#include "ui/main_window.h"

int main(int argc, char* argv[])
{
    cutefish::ui::Application app(argc, argv);
    app.setApplicationId("cutefish-photos");

    cutefish::photos::MainWindow window;
    window.show();

    return app.run();
}
