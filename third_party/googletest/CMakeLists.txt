set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/b")

file(MAKE_DIRECTORY "${BUILD_DIR}")

add_custom_target(
    build_googletest
    COMMAND
        ${CMAKE_COMMAND}
        "${SOURCE_DIR}"
        -G "${CMAKE_GENERATOR}"
        -DCMAKE_BUILD_TYPE=$<CONFIG>
        -Dgtest_force_shared_crt=ON
    COMMAND
        ${CMAKE_COMMAND}
        --build .
        --config $<CONFIG>
    WORKING_DIRECTORY "${BUILD_DIR}"
)
set_property(TARGET build_googletest PROPERTY FOLDER "third_party")

set(LIBRARY_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}gmock${CMAKE_STATIC_LIBRARY_SUFFIX}")
add_library(googletest STATIC IMPORTED GLOBAL)
set_target_properties(
    googletest
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${SOURCE_DIR}/googlemock/include;${SOURCE_DIR}/googletest/include"
    IMPORTED_LOCATION_DEBUG "${BUILD_DIR}/googlemock/Debug/${LIBRARY_NAME}"
    IMPORTED_LOCATION_RELEASE "${BUILD_DIR}/googlemock/Release/${LIBRARY_NAME}"
)

add_dependencies(googletest build_googletest)
