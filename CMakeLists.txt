cmake_minimum_required(VERSION 3.21)
project(LeoMiniGames VERSION 0.7.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Network Svg)
option(LEOMINIGAMES_ENABLE_AUDIO "Enable host audio (required for normal/release builds)" ON)
if(LEOMINIGAMES_ENABLE_AUDIO)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Multimedia)
endif()

qt_standard_project_setup(REQUIRES 6.8)
include(CTest)

set(LEOMINIGAMES_ANDROID_VERSION_CODE 703)
option(LEOMINIGAMES_PRIVACY_BUILD "Build the tracker-free/privacy-focused variant" OFF)
set(LEOMINIGAMES_APPLE_MOBILE_DEVICE_FAMILY "1,2" CACHE STRING
    "Apple mobile device family: 1=iPhone/iOS, 2=iPad/iPadOS, 1,2=universal")
set(LEOMINIGAMES_WINDOWS_CPU_PROFILE "baseline" CACHE STRING
    "Windows x64 CPU profile: baseline or avx2")
set_property(CACHE LEOMINIGAMES_WINDOWS_CPU_PROFILE PROPERTY STRINGS baseline avx2)

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        message(FATAL_ERROR "LeoMiniGames Qt 6 Windows release builds require 64-bit Windows (x86_64 or ARM64); 32-bit x86 is not an official release target.")
    endif()
    string(TOLOWER "${LEOMINIGAMES_WINDOWS_CPU_PROFILE}" LMG_WINDOWS_CPU_PROFILE_NORMALIZED)
    if(NOT LMG_WINDOWS_CPU_PROFILE_NORMALIZED STREQUAL "baseline" AND NOT LMG_WINDOWS_CPU_PROFILE_NORMALIZED STREQUAL "avx2")
        message(FATAL_ERROR "Invalid LEOMINIGAMES_WINDOWS_CPU_PROFILE='${LEOMINIGAMES_WINDOWS_CPU_PROFILE}'. Use baseline or avx2.")
    endif()
    if(LMG_WINDOWS_CPU_PROFILE_NORMALIZED STREQUAL "avx2")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(ARM64|arm64|aarch64)$")
            message(FATAL_ERROR "The AVX2 Windows profile is x86_64-only and cannot be used for ARM64.")
        endif()
        if(MSVC)
            add_compile_options(/arch:AVX2)
        else()
            add_compile_options(-mavx2)
        endif()
        add_compile_definitions(LMG_WINDOWS_X64_AVX2=1)
        message(STATUS "LeoMiniGames Windows CPU profile: x86_64 AVX2 optimized")
    else()
        message(STATUS "LeoMiniGames Windows CPU profile: baseline")
    endif()
endif()

# Bundle the small, BSD-licensed QtKeychain source to keep native credential
# stores available on every release target without runtime plaintext fallback.
function(lmg_add_keychain)
    set(BUILD_WITH_QT6 ON)
    set(BUILD_TESTING OFF)
    set(BUILD_TEST_APPLICATION OFF)
    set(BUILD_TRANSLATIONS OFF)
    set(BUILD_SHARED_LIBS OFF)
    if(UNIX AND NOT APPLE AND NOT ANDROID)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(LMG_LIBSECRET QUIET libsecret-1)
        endif()
        set(LIBSECRET_SUPPORT OFF)
        if(LMG_LIBSECRET_FOUND)
            set(LIBSECRET_SUPPORT ON)
        endif()
        if(NOT LMG_LIBSECRET_FOUND)
            message(STATUS "libsecret development files absent: credential storage uses KWallet/legacy keyring; plaintext fallback remains disabled")
        endif()
    endif()
    add_subdirectory(third_party/qtkeychain EXCLUDE_FROM_ALL)
endfunction()
lmg_add_keychain()
target_compile_definitions(qt6keychain PUBLIC QT6KEYCHAIN_STATIC_DEFINE)

file(GLOB LMG_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.h"
)

add_library(lmg_core STATIC ${LMG_CORE_SOURCES})
target_include_directories(lmg_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(lmg_core PUBLIC Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::Network Qt6::Svg qt6keychain)

qt_add_executable(LeoMiniGames
    src/main.cpp
    src/sdk/GameLocalStats.h
    src/sdk/IGamePlugin.h
    resources/resources.qrc
)

target_include_directories(LeoMiniGames PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(LeoMiniGames PRIVATE
    lmg_core
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Svg
    qt6keychain
)

if(LEOMINIGAMES_ENABLE_AUDIO AND TARGET Qt6::Multimedia)
    target_link_libraries(lmg_core PUBLIC Qt6::Multimedia)
    target_compile_definitions(lmg_core PUBLIC LMG_HAS_MULTIMEDIA=1)
else()
    message(STATUS "Audio explicitly disabled: audio services report unavailable")
endif()

if(LEOMINIGAMES_PRIVACY_BUILD)
    target_compile_definitions(lmg_core PUBLIC LMG_PRIVACY_BUILD=1)
endif()

add_subdirectory(plugins/builtin/Xox)
add_subdirectory(plugins/builtin/Blackjack)
add_subdirectory(plugins/builtin/Minesweeper)
add_subdirectory(plugins/builtin/TwentyFortyEight)
add_subdirectory(plugins/builtin/MemoryMatch)
add_subdirectory(plugins/builtin/ReactionTap)
add_subdirectory(qml)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

target_link_libraries(LeoMiniGames PRIVATE
    XoxPlugin
    BlackjackPlugin
    MinesweeperPlugin
    TwentyFortyEightPlugin
    MemoryMatchPlugin
    ReactionTapPlugin
)

if(ANDROID)
    if(NOT TARGET Qt6::QTlsBackendOpenSSLPlugin)
        message(FATAL_ERROR "The Android Qt kit must include the OpenSSL TLS backend plugin.")
    endif()
    qt_import_plugins(LeoMiniGames INCLUDE Qt6::QTlsBackendOpenSSLPlugin)
    set(LMG_ANDROID_OPENSSL_ROOT "$ENV{LMG_ANDROID_OPENSSL_ROOT}" CACHE PATH "Root of verified OpenSSL 3 Android libraries, with one subfolder per ABI")
    foreach(ssl_lib IN ITEMS crypto ssl)
        set(ssl_file "${LMG_ANDROID_OPENSSL_ROOT}/${CMAKE_ANDROID_ARCH_ABI}/lib${ssl_lib}_3.so")
        if(NOT EXISTS "${ssl_file}")
            message(FATAL_ERROR "Missing Android TLS library: ${ssl_file}. Run tools/ci/build_android_openssl.sh and set LMG_ANDROID_OPENSSL_ROOT.")
        endif()
        set_property(TARGET LeoMiniGames APPEND PROPERTY QT_ANDROID_EXTRA_LIBS "${ssl_file}")
    endforeach()
    target_link_options(LeoMiniGames PRIVATE "-Wl,-z,max-page-size=16384" "-Wl,-z,common-page-size=16384")
    set_target_properties(LeoMiniGames PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_PACKAGE_NAME "xyz.younglion.leominigames"
        QT_ANDROID_APP_NAME "LeoMiniGames"
        QT_ANDROID_APP_ICON "ic_launcher"
        QT_ANDROID_VERSION_CODE ${LEOMINIGAMES_ANDROID_VERSION_CODE}
        QT_ANDROID_VERSION_NAME "${PROJECT_VERSION}"
        QT_ANDROID_MIN_SDK_VERSION 28
        QT_ANDROID_TARGET_SDK_VERSION 36
        QT_ANDROID_COMPILE_SDK_VERSION 36
        QT_ANDROID_SDK_BUILD_TOOLS_REVISION "36.0.0"
    )
endif()

# macOS assets must never be injected into an iOS/iPadOS bundle.
if(APPLE AND NOT IOS)
    set(LEOMINIGAMES_MAC_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/branding/leominigames.icns")
    set_source_files_properties("${LEOMINIGAMES_MAC_ICON}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(LeoMiniGames PRIVATE "${LEOMINIGAMES_MAC_ICON}")
    set_target_properties(LeoMiniGames PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "xyz.younglion.leominigames"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_ICON_FILE "leominigames.icns"
    )
endif()

if(IOS)
    set_target_properties(LeoMiniGames PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "xyz.younglion.leominigames"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "${LEOMINIGAMES_APPLE_MOBILE_DEVICE_FAMILY}"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "xyz.younglion.leominigames"
    )

    # Online Qt iOS Multimedia packages use static FFmpeg libraries. Newer Qt
    # versions expose this helper to make those libraries part of the target.
    if(TARGET Qt6::Multimedia AND COMMAND qt_add_ios_ffmpeg_libraries)
        qt_add_ios_ffmpeg_libraries(LeoMiniGames)
    endif()
endif()

if(WIN32)
    set(LEOMINIGAMES_WIN_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/branding/leominigames.ico")
    configure_file(resources/windows/leominigames.rc.in "${CMAKE_CURRENT_BINARY_DIR}/leominigames.rc" @ONLY)
    target_sources(LeoMiniGames PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/leominigames.rc")
    set_target_properties(LeoMiniGames PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Desktop Linux packages and Flatpak share one install layout. Builtin plugins
# and QML are embedded in the executable; Qt itself remains a distro/runtime dependency.
if(UNIX AND NOT APPLE AND NOT ANDROID)
    include(GNUInstallDirs)
    install(TARGETS LeoMiniGames RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES packaging/linux/xyz.younglion.leominigames.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES packaging/linux/xyz.younglion.leominigames.metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
    install(FILES resources/branding/leominigames_icon_512.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps
        RENAME xyz.younglion.leominigames.png)
    install(FILES LICENSE NOTICE COPYRIGHT LICENSING.md LICENSE_HISTORY.md LICENSE_METADATA.json
        docs/THIRD_PARTY_NOTICES.md docs/QT_LGPL_COMPLIANCE.md
        DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/leominigames)
    install(FILES licenses/YOUNGLION_LMG_SDK_LICENSE_1.0.txt
        licenses/YOUNGLION_PACKAGE_LICENSE_1.0.txt
        DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/leominigames/licenses)
    # Minimal distro images often omit /usr/share/doc; licenses must survive.
    install(FILES LICENSE NOTICE COPYRIGHT LICENSE_METADATA.json
        licenses/YOUNGLION_LMG_SDK_LICENSE_1.0.txt licenses/YOUNGLION_PACKAGE_LICENSE_1.0.txt
        DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/leominigames)
endif()
