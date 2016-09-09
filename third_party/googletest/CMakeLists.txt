set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")

BuildCache_Get(
    # -1 for our build config, add extra things here (including generator expressions) to control cache sharing
    KEY "googletest-d225acc-${PLATFORM_NAME}-1"
    CACHE_DIR_VAR CACHE_DIR
)
set(CUSTOM_STAMP_FILE "${CACHE_DIR}/$<CONFIG>.success")

ExternalBuild_Add(
    build_googletest
    SOURCE_DIR "${SOURCE_DIR}"
    BUILD_DIR_VAR BUILD_DIR
    CUSTOM_STAMP_FILE "${CUSTOM_STAMP_FILE}"
)
set_property(TARGET build_googletest PROPERTY FOLDER "third_party")

ExternalBuild_Add_Step(
    NAME "Configure"
    COMMAND ${CMAKE_COMMAND}
        "${SOURCE_DIR}"
        -G "${CMAKE_GENERATOR}"
        -DCMAKE_BUILD_TYPE=$<CONFIG>
        "-DCMAKE_INSTALL_PREFIX=${CACHE_DIR}/$<CONFIG>"
        -Dgtest_force_shared_crt=ON
)

ExternalBuild_Add_Step(
    NAME "Build"
    COMMAND ${CMAKE_COMMAND}
        --build .
        --config $<CONFIG>
)

ExternalBuild_Add_Step(
    NAME "Install"
    COMMAND ${CMAKE_COMMAND}
        --build .
        --config $<CONFIG>
        --target INSTALL
)

ExternalBuild_Add_Step(
    NAME "Update Cache"
    COMMAND ${CMAKE_COMMAND} -E touch "${CUSTOM_STAMP_FILE}"
)

# CMake requires include paths be valid at configure time
set(INCLUDE_DIR "${CACHE_DIR}/$<CONFIG>/include")
file(GENERATE OUTPUT "${INCLUDE_DIR}/.eb" CONTENT "")

set(LIBRARY_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}gmock${CMAKE_STATIC_LIBRARY_SUFFIX}")
add_library(googletest STATIC IMPORTED GLOBAL)
set_target_properties(
    googletest
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CACHE_DIR}/$<CONFIG>/include"
    IMPORTED_LOCATION_DEBUG "${CACHE_DIR}/Debug/lib/${LIBRARY_NAME}"
    IMPORTED_LOCATION_RELEASE "${CACHE_DIR}/Release/lib/${LIBRARY_NAME}"
)

add_dependencies(googletest build_googletest)
