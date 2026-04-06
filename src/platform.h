#pragma once

// ============================================================================
// BrainSim Platform Detection & Abstractions
// Supports: Windows, Linux, macOS, iOS, Android
// ============================================================================

// --- MSVC / Windows: must be defined BEFORE any system includes ---
#if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
    #ifndef NOMINMAX
        #define NOMINMAX          // windows.h min/max macro'larini devre disi birak
    #endif
    #ifndef _USE_MATH_DEFINES
        #define _USE_MATH_DEFINES // M_PI vs. icin
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
#endif

// --- Platform detection ---
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
    #define BRAINSIM_WINDOWS 1
    #define BRAINSIM_DESKTOP 1
#elif defined(Q_OS_MACOS) || (defined(__APPLE__) && defined(__MACH__) && !defined(Q_OS_IOS))
    #define BRAINSIM_MACOS 1
    #define BRAINSIM_DESKTOP 1
#elif defined(Q_OS_IOS) || (defined(__APPLE__) && defined(TARGET_OS_IOS) && TARGET_OS_IOS)
    #define BRAINSIM_IOS 1
    #define BRAINSIM_MOBILE 1
#elif defined(Q_OS_ANDROID) || defined(__ANDROID__)
    #define BRAINSIM_ANDROID 1
    #define BRAINSIM_MOBILE 1
#elif defined(Q_OS_LINUX) || defined(__linux__)
    #define BRAINSIM_LINUX 1
    #define BRAINSIM_DESKTOP 1
#else
    #define BRAINSIM_UNKNOWN 1
    #define BRAINSIM_DESKTOP 1
#endif

// --- OpenGL variant selection ---
// Desktop: OpenGL 2.1 Compatibility (glBegin/glEnd legacy pipeline)
// iOS:     OpenGL ES 3.0 (or Metal via Qt abstraction)
// Android: OpenGL ES 3.0
#if defined(BRAINSIM_IOS) || defined(BRAINSIM_ANDROID)
    #define BRAINSIM_GLES 1
    #define BRAINSIM_GL_VERSION_MAJOR 3
    #define BRAINSIM_GL_VERSION_MINOR 0
#else
    #define BRAINSIM_GL_DESKTOP 1
    #define BRAINSIM_GL_VERSION_MAJOR 2
    #define BRAINSIM_GL_VERSION_MINOR 1
#endif

// --- OpenGL include selection ---
#if defined(BRAINSIM_GLES)
    #include <QOpenGLFunctions>
    #include <QOpenGLExtraFunctions>
    // On ES we cannot use glBegin/glEnd, must use VBOs
    #define BRAINSIM_USE_VBO 1
#else
    #include <QOpenGLFunctions_3_3_Core>
    // Desktop can use legacy immediate mode
    #define BRAINSIM_USE_LEGACY_GL 1
#endif

// --- Touch vs Mouse input ---
#if defined(BRAINSIM_MOBILE)
    #define BRAINSIM_TOUCH_INPUT 1
#else
    #define BRAINSIM_MOUSE_INPUT 1
#endif

// --- UI scaling ---
#if defined(BRAINSIM_MOBILE)
    #define BRAINSIM_DEFAULT_FONT_SIZE 18
    #define BRAINSIM_CONTROL_PANEL_WIDTH 320
    #define BRAINSIM_MIN_BUTTON_HEIGHT 48  // Finger-friendly
#else
    #define BRAINSIM_DEFAULT_FONT_SIZE 12
    #define BRAINSIM_CONTROL_PANEL_WIDTH 280
    #define BRAINSIM_MIN_BUTTON_HEIGHT 28
#endif

// --- File paths ---
#if defined(BRAINSIM_IOS)
    // iOS: use app bundle resources
    #define BRAINSIM_RESOURCE_PATH ":/resources/"
#elif defined(BRAINSIM_ANDROID)
    // Android: use assets
    #define BRAINSIM_RESOURCE_PATH "assets:/"
#elif defined(BRAINSIM_MACOS)
    // macOS: app bundle Resources
    #define BRAINSIM_RESOURCE_PATH "../Resources/"
#else
    // Windows/Linux: relative to executable
    #define BRAINSIM_RESOURCE_PATH "./"
#endif

// --- Performance tuning per platform ---
#if defined(BRAINSIM_MOBILE)
    #define BRAINSIM_MAX_VISIBLE_SYNAPSES 500
    #define BRAINSIM_SPHERE_SLICES 6
    #define BRAINSIM_SPHERE_STACKS 4
    #define BRAINSIM_BRAIN_SUBSTEPS 2
    #define BRAINSIM_TARGET_FPS 30
#else
    #define BRAINSIM_MAX_VISIBLE_SYNAPSES 2000
    #define BRAINSIM_SPHERE_SLICES 12
    #define BRAINSIM_SPHERE_STACKS 8
    #define BRAINSIM_BRAIN_SUBSTEPS 4
    #define BRAINSIM_TARGET_FPS 60
#endif

// --- Timer interval from target FPS ---
#define BRAINSIM_TIMER_INTERVAL_MS (1000 / BRAINSIM_TARGET_FPS)

// --- Haptic feedback (mobile only) ---
#if defined(BRAINSIM_IOS)
    // iOS haptic via UIImpactFeedbackGenerator (needs Obj-C bridge)
    #define BRAINSIM_HAS_HAPTICS 1
#elif defined(BRAINSIM_ANDROID)
    // Android haptic via QAndroidJniObject
    #define BRAINSIM_HAS_HAPTICS 1
#else
    #define BRAINSIM_HAS_HAPTICS 0
#endif

// --- Screen orientation ---
#if defined(BRAINSIM_MOBILE)
    #define BRAINSIM_DEFAULT_ORIENTATION Qt::LandscapeOrientation
#endif

// --- Compiler-specific ---
#if defined(_MSC_VER)
    #define BRAINSIM_MSVC 1
    #pragma warning(disable: 4244) // float/double conversion
    #pragma warning(disable: 4305) // double to float truncation
    #define _USE_MATH_DEFINES
#elif defined(__clang__)
    #define BRAINSIM_CLANG 1
#elif defined(__GNUC__)
    #define BRAINSIM_GCC 1
#endif

// --- M_PI fallback ---
#include <cmath>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
    #define M_PI_2 1.57079632679489661923
#endif
