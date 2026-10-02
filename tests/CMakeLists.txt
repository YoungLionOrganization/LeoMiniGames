# SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
add_executable(test_catalog_compat
    test_catalog_compat.cpp
    ${CMAKE_SOURCE_DIR}/src/CatalogCompat.cpp
)
target_include_directories(test_catalog_compat PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_catalog_compat PRIVATE Qt6::Core Qt6::Network)
add_test(NAME catalog_compat COMMAND test_catalog_compat)

add_executable(test_catalog_trust
    test_catalog_trust.cpp
    ${CMAKE_SOURCE_DIR}/src/CatalogTrustInfo.cpp
    ${CMAKE_SOURCE_DIR}/src/InstalledTrustSnapshot.cpp
)
target_include_directories(test_catalog_trust PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_catalog_trust PRIVATE Qt6::Core)
add_test(NAME catalog_trust COMMAND test_catalog_trust)

add_executable(test_game_runtime
    test_game_runtime.cpp
    ${CMAKE_SOURCE_DIR}/src/core/GameRuntime.cpp
    ${CMAKE_SOURCE_DIR}/src/core/GameRuntime.h
)
target_include_directories(test_game_runtime PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_game_runtime PRIVATE Qt6::Core)
add_test(NAME game_runtime_compat COMMAND test_game_runtime)

add_executable(test_publisher_trust_resolver
    test_publisher_trust_resolver.cpp
    ${CMAKE_SOURCE_DIR}/src/core/PublisherTrustResolver.cpp
    ${CMAKE_SOURCE_DIR}/src/core/PublisherTrustResolver.h
)
target_include_directories(test_publisher_trust_resolver PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_publisher_trust_resolver PRIVATE Qt6::Core)
add_test(NAME publisher_trust_resolver COMMAND test_publisher_trust_resolver)

set(COMPAT_FIXTURE_DIR ${CMAKE_SOURCE_DIR}/tests/fixtures/compat)
set(COMPAT_RCC_DIR ${CMAKE_CURRENT_BINARY_DIR}/compat-rcc)
file(MAKE_DIRECTORY ${COMPAT_RCC_DIR})
set(COMPAT_RCC_FILES)
foreach(name IN ITEMS v0_5_canonical v0_5_legacy_prefix v0_6_services v0_7_modern v0_7_migration unsupported_api malicious_legacy_namespace)
    file(GLOB_RECURSE fixture_inputs CONFIGURE_DEPENDS ${COMPAT_FIXTURE_DIR}/${name}/*)
    set(out ${COMPAT_RCC_DIR}/${name}.rcc)
    add_custom_command(
        OUTPUT ${out}
        COMMAND $<TARGET_FILE:Qt6::rcc> --binary ${COMPAT_FIXTURE_DIR}/${name}/mod.qrc -o ${out}
        DEPENDS
            ${fixture_inputs}
        WORKING_DIRECTORY ${COMPAT_FIXTURE_DIR}/${name}
        VERBATIM
    )
    list(APPEND COMPAT_RCC_FILES ${out})
endforeach()
add_custom_target(compat_rcc_fixtures ALL DEPENDS ${COMPAT_RCC_FILES})

add_executable(test_rcc_compat
    test_rcc_compat.cpp
    ${CMAKE_SOURCE_DIR}/src/core/RccPackageInspector.cpp
    ${CMAKE_SOURCE_DIR}/src/core/RccPackageInspector.h
)
target_include_directories(test_rcc_compat PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_rcc_compat PRIVATE Qt6::Core)
add_dependencies(test_rcc_compat compat_rcc_fixtures)
add_test(NAME rcc_backward_compat COMMAND test_rcc_compat ${COMPAT_RCC_DIR})

add_executable(test_save_safety test_save_safety.cpp
    ${CMAKE_SOURCE_DIR}/src/core/GameSave.cpp ${CMAKE_SOURCE_DIR}/src/core/GameSave.h
    ${CMAKE_SOURCE_DIR}/src/core/AppPaths.cpp ${CMAKE_SOURCE_DIR}/src/core/AppPaths.h)
target_include_directories(test_save_safety PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_save_safety PRIVATE Qt6::Core Qt6::Qml)
add_test(NAME save_safety COMMAND test_save_safety)

add_executable(test_network_safety test_network_safety.cpp)
target_include_directories(test_network_safety PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_network_safety PRIVATE Qt6::Core Qt6::Network)
add_test(NAME network_safety COMMAND test_network_safety)

add_executable(test_developer_manager test_developer_manager.cpp)
target_include_directories(test_developer_manager PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_developer_manager PRIVATE lmg_core Qt6::Core Qt6::Gui Qt6::Qml
    Qt6::Quick Qt6::QuickControls2 Qt6::Network Qt6::Svg qt6keychain)
if(TARGET Qt6::Multimedia)
    target_link_libraries(test_developer_manager PRIVATE Qt6::Multimedia)
    target_compile_definitions(test_developer_manager PRIVATE LMG_HAS_MULTIMEDIA=1)
endif()
add_test(NAME developer_auth_races COMMAND test_developer_manager)

add_executable(test_patch_runtime test_patch_runtime.cpp
    ${CMAKE_SOURCE_DIR}/plugins/builtin/MemoryMatch/MemoryMatchGame.cpp
    ${CMAKE_SOURCE_DIR}/plugins/builtin/MemoryMatch/MemoryMatchGame.h
    ${CMAKE_SOURCE_DIR}/plugins/builtin/ReactionTap/ReactionTapGame.cpp
    ${CMAKE_SOURCE_DIR}/plugins/builtin/ReactionTap/ReactionTapGame.h)
target_include_directories(test_patch_runtime PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/plugins/builtin)
target_link_libraries(test_patch_runtime PRIVATE lmg_core Qt6::Core Qt6::Gui Qt6::Quick)
add_test(NAME patch_runtime_regressions COMMAND test_patch_runtime)
set_tests_properties(patch_runtime_regressions PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

add_test(NAME builtin_qml_sessions COMMAND LeoMiniGames --smoke-test)
set_tests_properties(builtin_qml_sessions PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;XDG_DATA_HOME=${CMAKE_CURRENT_BINARY_DIR}/smoke-data;XDG_CONFIG_HOME=${CMAKE_CURRENT_BINARY_DIR}/smoke-config;XDG_CACHE_HOME=${CMAKE_CURRENT_BINARY_DIR}/smoke-cache"
    TIMEOUT 30)

add_executable(test_external_session test_external_session.cpp)
target_link_libraries(test_external_session PRIVATE lmg_core)
add_dependencies(test_external_session compat_rcc_fixtures)
add_test(NAME external_engine_session COMMAND test_external_session ${COMPAT_RCC_DIR})
set_tests_properties(external_engine_session PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

add_executable(test_catalog_operations test_catalog_operations.cpp)
target_link_libraries(test_catalog_operations PRIVATE lmg_core)
add_dependencies(test_catalog_operations compat_rcc_fixtures)
add_test(NAME catalog_operations COMMAND test_catalog_operations ${COMPAT_RCC_DIR})

add_test(NAME builtin_qml_landscape COMMAND LeoMiniGames --smoke-test)
set_tests_properties(builtin_qml_landscape PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;LMG_SMOKE_SIZE=640x360;XDG_DATA_HOME=${CMAKE_CURRENT_BINARY_DIR}/landscape-data;XDG_CONFIG_HOME=${CMAKE_CURRENT_BINARY_DIR}/landscape-config;XDG_CACHE_HOME=${CMAKE_CURRENT_BINARY_DIR}/landscape-cache"
    TIMEOUT 30)

add_executable(test_credential_order test_credential_order.cpp)
target_link_libraries(test_credential_order PRIVATE lmg_core)
add_test(NAME credential_store_order COMMAND test_credential_order)

set(THEME_RCC_DIR ${CMAKE_CURRENT_BINARY_DIR}/theme-rcc)
file(MAKE_DIRECTORY ${THEME_RCC_DIR})
set(THEME_RCC_FILES)
foreach(name IN ITEMS valid outside cycle bad_api bad_version)
    file(GLOB_RECURSE theme_inputs CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/fixtures/themes/${name}/*)
    set(out ${THEME_RCC_DIR}/${name}.rcc)
    add_custom_command(OUTPUT ${out}
        COMMAND $<TARGET_FILE:Qt6::rcc> --binary ${CMAKE_SOURCE_DIR}/tests/fixtures/themes/${name}/theme.qrc -o ${out}
        DEPENDS ${theme_inputs} VERBATIM)
    list(APPEND THEME_RCC_FILES ${out})
endforeach()
add_custom_target(theme_rcc_fixtures ALL DEPENDS ${THEME_RCC_FILES})
add_executable(test_theme_package test_theme_package.cpp ${CMAKE_SOURCE_DIR}/resources/resources.qrc)
target_link_libraries(test_theme_package PRIVATE lmg_core)
add_dependencies(test_theme_package theme_rcc_fixtures)
add_test(NAME theme_package_identity COMMAND test_theme_package ${THEME_RCC_DIR})

add_executable(test_builtin_persistence test_builtin_persistence.cpp)
target_include_directories(test_builtin_persistence PRIVATE ${CMAKE_SOURCE_DIR}/plugins/builtin)
target_link_libraries(test_builtin_persistence PRIVATE lmg_core XoxPlugin BlackjackPlugin MinesweeperPlugin TwentyFortyEightPlugin MemoryMatchPlugin ReactionTapPlugin)
add_test(NAME builtin_state_persistence COMMAND test_builtin_persistence)
set_tests_properties(builtin_state_persistence PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 30)

# Dynamic plugin discovery can be tested without contacting production servers.
get_target_property(LMG_QT_NETWORK_TYPE Qt6::Network TYPE)
if(NOT ANDROID AND NOT IOS AND LMG_QT_NETWORK_TYPE STREQUAL "SHARED_LIBRARY")
    add_executable(test_tls_runtime test_tls_runtime.cpp)
    target_link_libraries(test_tls_runtime PRIVATE lmg_core)
    add_test(NAME tls_backend_discovery COMMAND test_tls_runtime)
endif()

add_executable(test_audio test_audio.cpp
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc
    ${CMAKE_SOURCE_DIR}/tests/fixtures/audio/audio.qrc)
target_link_libraries(test_audio PRIVATE lmg_core)
add_test(NAME host_audio_compatibility COMMAND test_audio)
set_tests_properties(host_audio_compatibility PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 30)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(SDK_RCC_DIR ${CMAKE_CURRENT_BINARY_DIR}/sdk-rcc)
file(MAKE_DIRECTORY ${SDK_RCC_DIR})
set(SDK_RCC_FILES)
foreach(example IN ITEMS ExampleHelloMod ExampleModernMod ExampleTheme)
    if(example STREQUAL "ExampleTheme")
        set(sdk_project ${CMAKE_SOURCE_DIR}/theme-sdk/${example})
    else()
        set(sdk_project ${CMAKE_SOURCE_DIR}/mod-sdk/${example})
    endif()
    file(GLOB_RECURSE sdk_inputs CONFIGURE_DEPENDS ${sdk_project}/*)
    list(FILTER sdk_inputs EXCLUDE REGEX "/build/")
    set(out ${SDK_RCC_DIR}/${example}.rcc)
    add_custom_command(OUTPUT ${out}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/sdk/build_package.py
            ${sdk_project} --output ${out} --rcc $<TARGET_FILE:Qt6::rcc>
        DEPENDS ${sdk_inputs} ${CMAKE_SOURCE_DIR}/tools/sdk/build_package.py VERBATIM)
    list(APPEND SDK_RCC_FILES ${out})
endforeach()
add_custom_target(sdk_rcc_examples ALL DEPENDS ${SDK_RCC_FILES})
add_executable(test_sdk_examples test_sdk_examples.cpp ${CMAKE_SOURCE_DIR}/resources/resources.qrc)
target_link_libraries(test_sdk_examples PRIVATE lmg_core)
add_dependencies(test_sdk_examples sdk_rcc_examples)
add_test(NAME sdk_examples COMMAND test_sdk_examples ${SDK_RCC_DIR})
set_tests_properties(sdk_examples PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software" TIMEOUT 30)

# The native SDK is independently buildable; compile it on each desktop test kit.
# This does not load the module or grant the host's native trust permission.
if(NOT ANDROID AND NOT IOS)
    add_subdirectory(${CMAKE_SOURCE_DIR}/native-sdk/ExampleNativeGame ${CMAKE_CURRENT_BINARY_DIR}/native-sdk-example)
endif()
