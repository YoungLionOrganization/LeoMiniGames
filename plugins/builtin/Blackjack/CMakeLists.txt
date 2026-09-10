qt_add_plugin(BlackjackPlugin
    STATIC
    CLASS_NAME BlackjackPlugin
    BlackjackPlugin.cpp
    BlackjackPlugin.h
    BlackjackGame.cpp
    BlackjackGame.h
    metadata.json
)

target_include_directories(BlackjackPlugin PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(BlackjackPlugin PRIVATE Qt6::Core)
