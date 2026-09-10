qt_add_plugin(XoxPlugin
    STATIC
    CLASS_NAME XoxPlugin
    XoxPlugin.cpp
    XoxPlugin.h
    XoxGame.cpp
    XoxGame.h
    metadata.json
)

target_include_directories(XoxPlugin PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(XoxPlugin PRIVATE Qt6::Core)
