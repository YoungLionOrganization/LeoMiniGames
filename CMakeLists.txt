cmake_minimum_required(VERSION 3.21)
project(LeoMiniGames VERSION 0.7.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Network Svg)
find_package(Qt6 6.5 QUIET COMPONENTS Multimedia)
qt_standard_project_setup(REQUIRES 6.5)
include(CTest)

set(LEOMINIGAMES_ANDROID_VERSION_CODE 700)
option(LEOMINIGAMES_FDROID "Build the tracker-free F-Droid variant" OFF)

file(GLOB LMG_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.h"
)

qt_add_executable(LeoMiniGames
    src/main.cpp
    ${LMG_CORE_SOURCES}
    src/sdk/GameLocalStats.h
    src/sdk/IGamePlugin.h
    resources/resources.qrc
)

target_include_directories(LeoMiniGames PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(LeoMiniGames PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Svg
)

if(TARGET Qt6::Multimedia)
    target_link_libraries(LeoMiniGames PRIVATE Qt6::Multimedia)
    target_compile_definitions(LeoMiniGames PRIVATE LMG_HAS_MULTIMEDIA=1)
else()
    message(STATUS "Qt Multimedia not found: audio services will use a silent capability fallback")
endif()

if(LEOMINIGAMES_FDROID)
    target_compile_definitions(LeoMiniGames PRIVATE LMG_FDROID_BUILD=1)
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
    set_target_properties(LeoMiniGames PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_VERSION_CODE ${LEOMINIGAMES_ANDROID_VERSION_CODE}
        QT_ANDROID_VERSION_NAME "${PROJECT_VERSION}"
    )
endif()

if(APPLE)
    set_target_properties(LeoMiniGames PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "xyz.younglion.leominigames"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()

if(WIN32)
    set_target_properties(LeoMiniGames PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

