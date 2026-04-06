# ==============================================================================
# BrainSim - C. elegans Connectome Whole-Brain Emulation
# Qt 6.10+ Project File
# Platforms: Windows, Linux, macOS, iOS, Android
# ==============================================================================

QT += core gui widgets opengl openglwidgets

CONFIG += c++17
DEFINES += QT_DEPRECATED_WARNINGS

TARGET = BrainSim
TEMPLATE = app

# ==============================================================================
# Source Files
# ==============================================================================

INCLUDEPATH += src

HEADERS += \
    src/platform.h \
    src/brain/neuron.h \
    src/brain/synapse.h \
    src/brain/connectome_data.h \
    src/brain/brain.h \
    src/creature/creature.h \
    src/creature/leg.h \
    src/creature/sensor.h \
    src/world/world.h \
    src/world/food.h \
    src/world/obstacle.h \
    src/renderer/creature_renderer.h \
    src/renderer/world_renderer.h \
    src/renderer/brain_renderer.h \
    src/glwidget.h \
    src/mainwindow.h

SOURCES += \
    src/main.cpp \
    src/brain/neuron.cpp \
    src/brain/connectome_data.cpp \
    src/brain/brain.cpp \
    src/creature/creature.cpp \
    src/creature/leg.cpp \
    src/creature/sensor.cpp \
    src/world/world.cpp \
    src/renderer/creature_renderer.cpp \
    src/renderer/world_renderer.cpp \
    src/renderer/brain_renderer.cpp \
    src/glwidget.cpp \
    src/mainwindow.cpp

# ==============================================================================
# Platform-specific configuration
# ==============================================================================

# --- Windows ---
win32 {
    DEFINES += BRAINSIM_WINDOWS BRAINSIM_DESKTOP
    LIBS += -lopengl32 -lglu32 -luser32

    # MSVC specific
    msvc {
        QMAKE_CXXFLAGS += /W3 /MP
        DEFINES += _USE_MATH_DEFINES
        DEFINES += _CRT_SECURE_NO_WARNINGS
    }

    # MinGW specific
    mingw {
        QMAKE_CXXFLAGS += -Wall -Wextra
        LIBS += -lmingw32
    }

    # Windows application icon
    # RC_ICONS = resources/brainsim.ico

    # Version info
    VERSION = 1.0.0
    QMAKE_TARGET_COMPANY = "Maren Robotics"
    QMAKE_TARGET_PRODUCT = "BrainSim"
    QMAKE_TARGET_DESCRIPTION = "C. elegans Connectome Simulation"
    QMAKE_TARGET_COPYRIGHT = "2026 Maren Robotics"
}

# --- Linux ---
unix:!macx:!ios:!android {
    DEFINES += BRAINSIM_LINUX BRAINSIM_DESKTOP
    LIBS += -lGL -lGLU

    QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic

    # Install paths
    target.path = /usr/local/bin
    INSTALLS += target

    # Desktop entry
    desktop.path = /usr/share/applications
    desktop.files = resources/brainsim.desktop
    INSTALLS += desktop
}

# --- macOS ---
macx:!ios {
    DEFINES += BRAINSIM_MACOS BRAINSIM_DESKTOP
    LIBS += -framework OpenGL

    QMAKE_CXXFLAGS += -Wall -Wextra
    # Suppress OpenGL deprecation warnings on macOS
    QMAKE_CXXFLAGS += -Wno-deprecated-declarations

    # App bundle
    QMAKE_INFO_PLIST = platform/macos/Info.plist

    # macOS icon
    # ICON = resources/brainsim.icns

    # Bundle identifier
    QMAKE_TARGET_BUNDLE_PREFIX = com.tbiliyor
    QMAKE_BUNDLE = BrainSim

    # Minimum deployment target
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 12.0
}

# --- iOS ---
ios {
    DEFINES += BRAINSIM_IOS BRAINSIM_MOBILE BRAINSIM_GLES

    # OpenGL ES
    LIBS += -framework OpenGLES
    LIBS += -framework UIKit
    LIBS += -framework CoreGraphics
    LIBS += -framework QuartzCore

    # Qt modules for iOS
    QT += sensors

    # Info.plist
    QMAKE_INFO_PLIST = platform/ios/Info.plist

    # Bundle identifier
    QMAKE_TARGET_BUNDLE_PREFIX = com.tbiliyor
    QMAKE_BUNDLE = BrainSim

    # Deployment target
    QMAKE_IOS_DEPLOYMENT_TARGET = 15.0

    # Device orientation
    # Handled in Info.plist: UISupportedInterfaceOrientations = Landscape

    # App icon and launch screen
    # QMAKE_ASSET_CATALOGS += resources/ios/Assets.xcassets
    # app_launch_screen.files = platform/ios/LaunchScreen.storyboard
    # QMAKE_BUNDLE_DATA += app_launch_screen

    # Disable bitcode (deprecated)
    QMAKE_APPLE_DEVICE_ARCHS = arm64
}

# --- Android ---
android {
    DEFINES += BRAINSIM_ANDROID BRAINSIM_MOBILE BRAINSIM_GLES

    # OpenGL ES
    LIBS += -lGLESv2 -lEGL

    # Qt modules for Android
    QT += sensors

    # Android manifest
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/platform/android

    # Target SDK
    ANDROID_TARGET_SDK_VERSION = 34
    ANDROID_MIN_SDK_VERSION = 24

    # Architecture
    ANDROID_ABIS = arm64-v8a armeabi-v7a x86_64
}

# ==============================================================================
# Build output directories
# ==============================================================================

CONFIG(debug, debug|release) {
    DESTDIR = $$OUT_PWD/debug
    OBJECTS_DIR = $$OUT_PWD/debug/.obj
    MOC_DIR = $$OUT_PWD/debug/.moc
    RCC_DIR = $$OUT_PWD/debug/.rcc
} else {
    DESTDIR = $$OUT_PWD/release
    OBJECTS_DIR = $$OUT_PWD/release/.obj
    MOC_DIR = $$OUT_PWD/release/.moc
    RCC_DIR = $$OUT_PWD/release/.rcc
}

# ==============================================================================
# Resources (if any)
# ==============================================================================

# RESOURCES += resources/resources.qrc

# ==============================================================================
# Performance flags for release builds
# ==============================================================================

CONFIG(release, debug|release) {
    win32-msvc* {
        QMAKE_CXXFLAGS_RELEASE += /O2 /GL /arch:AVX2
        QMAKE_LFLAGS_RELEASE += /LTCG
    }
    unix {
        QMAKE_CXXFLAGS_RELEASE += -O3 -march=native -flto
        QMAKE_LFLAGS_RELEASE += -flto
    }
    macx {
        QMAKE_CXXFLAGS_RELEASE += -O3 -flto
        QMAKE_LFLAGS_RELEASE += -flto
    }
}
