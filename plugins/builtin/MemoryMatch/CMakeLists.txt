qt_add_plugin(MemoryMatchPlugin
    STATIC
    CLASS_NAME MemoryMatchPlugin
    MemoryMatchPlugin.cpp
    MemoryMatchPlugin.h
    MemoryMatchGame.cpp
    MemoryMatchGame.h
    metadata.json
)

target_include_directories(MemoryMatchPlugin PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(MemoryMatchPlugin PRIVATE Qt6::Core)
