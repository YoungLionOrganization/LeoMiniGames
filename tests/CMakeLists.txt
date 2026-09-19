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
foreach(name IN ITEMS v0_5_canonical v0_5_legacy_prefix v0_6_services v0_7_modern malicious_legacy_namespace)
    set(out ${COMPAT_RCC_DIR}/${name}.rcc)
    add_custom_command(
        OUTPUT ${out}
        COMMAND $<TARGET_FILE:Qt6::rcc> --binary ${COMPAT_FIXTURE_DIR}/${name}/mod.qrc -o ${out}
        DEPENDS
            ${COMPAT_FIXTURE_DIR}/${name}/mod.qrc
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
