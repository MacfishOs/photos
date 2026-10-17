/*
 * Copyright (C) 2021 CutefishOS Team.
 * Copyright (C) 2026 Project Aevis Contributors.
 *
 * Project Aevis Photos Application Entry Point
 */

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QDir>
#include <QCommandLineParser>
#include <QPixmapCache>
#include <QTimer>
#ifdef __GLIBC__
#include <malloc.h>
#endif

#include "photosapp.h"
#include "photomodel.h"
#include "foldermodel.h"
#include "imageprovider.h"

int main(int argc, char *argv[])
{
#ifdef __GLIBC__
    mallopt(M_ARENA_MAX, 1);
    mallopt(M_TRIM_THRESHOLD, 64 * 1024);
    mallopt(M_MMAP_THRESHOLD, 32 * 1024);
#endif

    // Dynamic EGL vendor selection to prevent NVIDIA driver bloat on Intel/AMD GPUs
    if (!QFile::exists(QStringLiteral("/dev/nvidia0")) &&
        QFile::exists(QStringLiteral("/usr/share/glvnd/egl_vendor.d/50_mesa.json"))) {
        qputenv("__EGL_VENDOR_LIBRARY_FILENAMES", "/usr/share/glvnd/egl_vendor.d/50_mesa.json");
    }

    qputenv("QSG_RENDER_LOOP", "basic");

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif

    QApplication app(argc, argv);
    QPixmapCache::setCacheLimit(2048);

    app.setApplicationName(QStringLiteral("aevis-photos"));
    app.setDesktopFileName(QStringLiteral("aevis-photos"));
    app.setApplicationDisplayName(QStringLiteral("Photos"));
    app.setOrganizationName(QStringLiteral("aevis"));
    app.setOrganizationDomain(QStringLiteral("aevis.org"));

    QIcon appIcon = QIcon::fromTheme(QStringLiteral("aevis-photos"), QIcon::fromTheme(QStringLiteral("cutefish-photos"), QIcon(QStringLiteral(":/images/cutefish-photos.svg"))));
    app.setWindowIcon(appIcon);

    // Parse command line arguments (e.g. image file to open)
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Project Aevis Photos and Image Viewer"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "Image file to view");
    parser.process(app);

    QString initialImagePath;
    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        QString arg = args.first();
        if (arg.startsWith(QLatin1String("file:"), Qt::CaseInsensitive)) {
            arg = QUrl(arg).toLocalFile();
        }
        QFileInfo fi(arg);
        if (fi.exists() && fi.isFile()) {
            initialImagePath = fi.canonicalFilePath();
            if (initialImagePath.isEmpty()) {
                initialImagePath = fi.absoluteFilePath();
            }
        }
    }

    PhotosApp *photosApp = new PhotosApp(&app);

    for (const char *uri : {"Cutefish.Photos", "Aevis.Photos"}) {
        qmlRegisterType<PhotoModel>(uri, 1, 0, "PhotoModel");
        qmlRegisterType<FolderModel>(uri, 1, 0, "FolderModel");
        qmlRegisterSingletonInstance(uri, 1, 0, "PhotosApp", photosApp);
    }

    QQmlApplicationEngine engine;

    engine.addImageProvider(QStringLiteral("photothumb"), new ThumbnailProvider());

    // Translations
    QLocale locale;
    QString qmFilePath = QStringLiteral("/usr/share/aevis-photos/translations/%1.qm").arg(locale.name());
    if (!QFile::exists(qmFilePath)) {
        qmFilePath = QStringLiteral("/usr/share/cutefish-photos/translations/%1.qm").arg(locale.name());
    }
    if (QFile::exists(qmFilePath)) {
        QTranslator *translator = new QTranslator(&app);
        if (translator->load(qmFilePath)) {
            QApplication::installTranslator(translator);
        } else {
            translator->deleteLater();
        }
    }

    engine.rootContext()->setContextProperty("initialImagePath", initialImagePath);

    engine.addImportPath(QStringLiteral("qrc:/"));
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    // Trim freed memory and unused QML components after startup and periodically
    QTimer::singleShot(2000, [&engine]() {
        engine.trimComponentCache();
#ifdef __GLIBC__
        malloc_trim(0);
#endif
    });

    QTimer *trimTimer = new QTimer(&app);
    trimTimer->setInterval(45000);
    QObject::connect(trimTimer, &QTimer::timeout, [&engine]() {
        engine.trimComponentCache();
#ifdef __GLIBC__
        malloc_trim(0);
#endif
    });
    trimTimer->start();

    return app.exec();
}
