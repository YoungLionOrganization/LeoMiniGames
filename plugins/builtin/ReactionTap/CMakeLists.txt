qt_add_plugin(ReactionTapPlugin
    STATIC
    CLASS_NAME ReactionTapPlugin
    ReactionTapPlugin.cpp
    ReactionTapPlugin.h
    ReactionTapGame.cpp
    ReactionTapGame.h
    metadata.json
)

target_include_directories(ReactionTapPlugin PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(ReactionTapPlugin PRIVATE Qt6::Core)
