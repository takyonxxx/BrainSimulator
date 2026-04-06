#include "platform.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QScreen>
#include "mainwindow.h"

#if defined(BRAINSIM_IOS)
    #include <QGuiApplication>
#endif

#if defined(BRAINSIM_ANDROID)
    #include <QGuiApplication>
    #include <QAndroidJniObject>  // Qt6: may need QtCore/private
#endif

#if defined(BRAINSIM_WINDOWS)
    // DPI awareness on Windows
    #include <windows.h>
#endif

// ============================================================================
// Dark theme stylesheet (common across all platforms)
// ============================================================================
static const char* darkStyleSheet()
{
    return
        "QMainWindow, QWidget { background-color: #1e1e2e; color: #cdd6f4; }"
        "QGroupBox { border: 1px solid #45475a; border-radius: 6px; margin-top: 8px; "
        "  padding-top: 16px; font-weight: bold; color: #89b4fa; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        "QPushButton { background: #45475a; color: #cdd6f4; border: 1px solid #585b70; "
        "  border-radius: 4px; padding: 6px 12px; "
#if defined(BRAINSIM_MOBILE)
        "  min-height: 48px; font-size: 16px; "
#endif
        "}"
        "QPushButton:hover { background: #585b70; }"
        "QPushButton:pressed { background: #313244; }"
        "QSlider::groove:horizontal { background: #45475a; height: 6px; border-radius: 3px; }"
        "QSlider::handle:horizontal { background: #89b4fa; width: 16px; height: 16px; "
        "  border-radius: 8px; margin: -5px 0; "
#if defined(BRAINSIM_MOBILE)
        "  width: 28px; height: 28px; border-radius: 14px; margin: -11px 0; "
#endif
        "}"
        "QCheckBox { spacing: 6px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; "
#if defined(BRAINSIM_MOBILE)
        "  width: 24px; height: 24px; "
#endif
        "}"
        "QComboBox { background: #45475a; border: 1px solid #585b70; border-radius: 4px; "
        "  padding: 4px 8px; color: #cdd6f4; "
#if defined(BRAINSIM_MOBILE)
        "  min-height: 44px; font-size: 16px; "
#endif
        "}"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background: #313244; color: #cdd6f4; "
        "  selection-background-color: #45475a; }"
        "QStatusBar { background: #181825; color: #a6adc8; font-size: 11px; }"
        "QLabel { font-size: "
#if defined(BRAINSIM_MOBILE)
        "16"
#else
        "12"
#endif
        "px; }"
        "QScrollArea { border: none; }"
    ;
}

// ============================================================================
// Main
// ============================================================================
int main(int argc, char* argv[])
{
#if defined(BRAINSIM_WINDOWS)
    // Enable DPI awareness on Windows 10+
    SetProcessDPIAware();
#endif

#if defined(BRAINSIM_MACOS)
    // macOS: supress OpenGL deprecation warnings at runtime
    qputenv("QT_MAC_WANTS_LAYER", "1");
#endif

    // --- Surface format (before QApplication) ---
    QSurfaceFormat fmt;

#if defined(BRAINSIM_GLES)
    // iOS / Android: OpenGL ES
    fmt.setRenderableType(QSurfaceFormat::OpenGLES);
    fmt.setVersion(BRAINSIM_GL_VERSION_MAJOR, BRAINSIM_GL_VERSION_MINOR);
    fmt.setDepthBufferSize(24);
    fmt.setSamples(4);
#elif defined(BRAINSIM_MACOS)
    // macOS: Compatibility profile for legacy GL
    fmt.setVersion(2, 1);
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    fmt.setDepthBufferSize(24);
    fmt.setSamples(4);
    fmt.setSwapInterval(1);
#elif defined(BRAINSIM_WINDOWS)
    fmt.setVersion(2, 1);
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    fmt.setDepthBufferSize(24);
    fmt.setSamples(4);
    fmt.setSwapInterval(1);
#elif defined(BRAINSIM_LINUX)
    fmt.setVersion(2, 1);
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    fmt.setDepthBufferSize(24);
    fmt.setSamples(4);
    fmt.setSwapInterval(1);
#else
    fmt.setVersion(2, 1);
    fmt.setDepthBufferSize(24);
    fmt.setSamples(4);
#endif

    QSurfaceFormat::setDefaultFormat(fmt);

    // --- Create application ---
    QApplication app(argc, argv);
    app.setApplicationName("BrainSim");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("MarenRobotics");
    app.setOrganizationDomain("com.tbiliyor.brainsim");

    // Apply dark theme
    app.setStyleSheet(darkStyleSheet());

#if defined(BRAINSIM_MOBILE)
    // Force landscape on mobile
    // (handled via Info.plist on iOS, AndroidManifest on Android)
#endif

    // --- Create and show window ---
    MainWindow window;

#if defined(BRAINSIM_MOBILE)
    // Fullscreen on mobile
    window.showFullScreen();
#elif defined(BRAINSIM_MACOS)
    // macOS: reasonable default size
    window.resize(1400, 900);
    window.show();
#elif defined(BRAINSIM_WINDOWS)
    window.resize(1400, 900);
    window.show();
#elif defined(BRAINSIM_LINUX)
    window.resize(1400, 900);
    window.show();
#else
    window.resize(1400, 900);
    window.show();
#endif

    return app.exec();
}
