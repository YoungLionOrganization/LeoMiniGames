qt_add_plugin(MinesweeperPlugin
    STATIC
    CLASS_NAME MinesweeperPlugin
    MinesweeperPlugin.cpp
    MinesweeperPlugin.h
    MinesweeperGame.cpp
    MinesweeperGame.h
    metadata.json
)

target_include_directories(MinesweeperPlugin PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(MinesweeperPlugin PRIVATE Qt6::Core)
