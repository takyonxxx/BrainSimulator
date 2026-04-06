# ==============================================================================
# BrainSim - C. elegans Connectome Whole-Brain Emulation
# CMake Build - Windows, Linux, macOS, iOS, Android
# ==============================================================================
cmake_minimum_required(VERSION 3.16)
project(BrainSim VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ==============================================================================
# Qt6 Modules
# ==============================================================================
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

if(IOS OR ANDROID)
    find_package(Qt6 COMPONENTS Sensors QUIET)
endif()

# ==============================================================================
# Sources
# ==============================================================================
set(BRAIN_SOURCES
    src/brain/neuron.cpp
    src/brain/connectome_data.cpp
    src/brain/brain.cpp
)

set(BRAIN_HEADERS
    src/platform.h
    src/brain/neuron.h
    src/brain/synapse.h
    src/brain/connectome_data.h
    src/brain/brain.h
)

set(CREATURE_SOURCES
    src/creature/creature.cpp
    src/creature/leg.cpp
    src/creature/sensor.cpp
)

set(CREATURE_HEADERS
    src/creature/creature.h
    src/creature/leg.h
    src/creature/sensor.h
)

set(WORLD_SOURCES
    src/world/world.cpp
)

set(WORLD_HEADERS
    src/world/world.h
    src/world/food.h
    src/world/obstacle.h
)

set(RENDERER_SOURCES
    src/renderer/creature_renderer.cpp
    src/renderer/world_renderer.cpp
    src/renderer/brain_renderer.cpp
)

set(RENDERER_HEADERS
    src/renderer/creature_renderer.h
    src/renderer/world_renderer.h
    src/renderer/brain_renderer.h
)

set(APP_SOURCES
    src/main.cpp
    src/glwidget.cpp
    src/mainwindow.cpp
)

set(APP_HEADERS
    src/glwidget.h
    src/mainwindow.h
)

set(ALL_SOURCES
    ${BRAIN_SOURCES} ${CREATURE_SOURCES} ${WORLD_SOURCES}
    ${RENDERER_SOURCES} ${APP_SOURCES}
)

set(ALL_HEADERS
    ${BRAIN_HEADERS} ${CREATURE_HEADERS} ${WORLD_HEADERS}
    ${RENDERER_HEADERS} ${APP_HEADERS}
)

# ==============================================================================
# Executable
# ==============================================================================
if(ANDROID)
    add_library(BrainSim SHARED ${ALL_SOURCES} ${ALL_HEADERS})
elseif(IOS)
    add_executable(BrainSim MACOSX_BUNDLE ${ALL_SOURCES} ${ALL_HEADERS})
elseif(APPLE)
    add_executable(BrainSim MACOSX_BUNDLE ${ALL_SOURCES} ${ALL_HEADERS})
elseif(WIN32)
    add_executable(BrainSim WIN32 ${ALL_SOURCES} ${ALL_HEADERS})
else()
    add_executable(BrainSim ${ALL_SOURCES} ${ALL_HEADERS})
endif()

target_include_directories(BrainSim PRIVATE src)

# ==============================================================================
# Platform-specific linking
# ==============================================================================
target_link_libraries(BrainSim PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

if(TARGET Qt6::Sensors)
    target_link_libraries(BrainSim PRIVATE Qt6::Sensors)
endif()

# --- Windows ---
if(WIN32)
    target_compile_definitions(BrainSim PRIVATE BRAINSIM_WINDOWS BRAINSIM_DESKTOP)
    target_link_libraries(BrainSim PRIVATE opengl32 glu32 user32)

    if(MSVC)
        target_compile_definitions(BrainSim PRIVATE _USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS)
        target_compile_options(BrainSim PRIVATE /W3 /MP)
    endif()
endif()

# --- Linux ---
if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_compile_definitions(BrainSim PRIVATE BRAINSIM_LINUX BRAINSIM_DESKTOP)
    find_package(OpenGL REQUIRED)
    target_link_libraries(BrainSim PRIVATE OpenGL::GL OpenGL::GLU)
    target_compile_options(BrainSim PRIVATE -Wall -Wextra)

    install(TARGETS BrainSim DESTINATION bin)
endif()

# --- macOS ---
if(APPLE AND NOT IOS)
    target_compile_definitions(BrainSim PRIVATE BRAINSIM_MACOS BRAINSIM_DESKTOP)
    target_link_libraries(BrainSim PRIVATE "-framework OpenGL")
    target_compile_options(BrainSim PRIVATE -Wall -Wextra -Wno-deprecated-declarations)

    set_target_properties(BrainSim PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/platform/macos/Info.plist
        MACOSX_BUNDLE_BUNDLE_NAME "BrainSim"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.tbiliyor.brainsim"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    )
endif()

# --- iOS ---
if(IOS)
    target_compile_definitions(BrainSim PRIVATE BRAINSIM_IOS BRAINSIM_MOBILE BRAINSIM_GLES)
    target_link_libraries(BrainSim PRIVATE
        "-framework OpenGLES"
        "-framework UIKit"
        "-framework CoreGraphics"
        "-framework QuartzCore"
    )

    set_target_properties(BrainSim PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/platform/ios/Info.plist
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.tbiliyor.brainsim"
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "15.0"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
    )
endif()

# --- Android ---
if(ANDROID)
    target_compile_definitions(BrainSim PRIVATE BRAINSIM_ANDROID BRAINSIM_MOBILE BRAINSIM_GLES)
    target_link_libraries(BrainSim PRIVATE GLESv2 EGL)

    set_target_properties(BrainSim PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/platform/android
    )
endif()

# ==============================================================================
# Release optimizations
# ==============================================================================
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(BrainSim PRIVATE /O2 /GL)
    elseif(UNIX)
        target_compile_options(BrainSim PRIVATE -O3 -flto)
        target_link_options(BrainSim PRIVATE -flto)
        if(NOT APPLE AND NOT ANDROID AND NOT IOS)
            target_compile_options(BrainSim PRIVATE -march=native)
        endif()
    endif()
endif()
