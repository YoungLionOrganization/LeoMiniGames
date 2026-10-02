cmake_minimum_required(VERSION 3.21)
project(ExampleNativeGame VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Qml Quick)
qt_add_plugin(ExampleNativeGame SHARED
    CLASS_NAME ExampleNativePlugin
    ExampleNativePlugin.h ExampleNativePlugin.cpp native.qrc)
target_include_directories(ExampleNativeGame PRIVATE ../../src/sdk)
target_link_libraries(ExampleNativeGame PRIVATE Qt6::Core Qt6::Qml Qt6::Quick)
