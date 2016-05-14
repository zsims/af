set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")

BuildCache_Get(
    # -1 for our build config, add extra things here to control cache sharing
    KEY "boost-1.60.0-${PLATFORM_NAME}-1"
    CACHE_DIR_VAR CACHE_DIR
)

# Run build off this stamp (the cache)
set(CUSTOM_STAMP_FILE "${CACHE_DIR}/eb.success")

ExternalBuild_Add(
    build_boost
    SOURCE_DIR "${SOURCE_DIR}"
    BUILD_DIR_VAR BUILD_DIR
    CUSTOM_STAMP_FILE "${CUSTOM_STAMP_FILE}"
)
set_property(TARGET build_boost PROPERTY FOLDER "third_party")

ExternalBuild_Add_Step(
    NAME "Configure"
    COMMAND "bootstrap.bat"
    WORKING_DIRECTORY "${SOURCE_DIR}"
)

ExternalBuild_Add_Step(
    NAME "Build"
    COMMAND "b2.exe"
        -j8
        --layout=tagged
        --with-system
        --with-filesystem
        --build-dir=${BUILD_DIR}
        --prefix=${CACHE_DIR}
        link=static
        threading=multi
        toolset=msvc-14.0
        runtime-link=shared
        # Note the architecture for 64-bit is still x86, see http://www.boost.org/build/tutorial.html
        architecture=x86
        address-model=$<$<STREQUAL:x86,${PLATFORM_NAME}>:32>$<$<STREQUAL:x64,${PLATFORM_NAME}>:64>
        define=BOOST_CB_DISABLE_DEBUG
        variant=$<LOWER_CASE:$<CONFIG>>
        install
    WORKING_DIRECTORY "${SOURCE_DIR}"
)

ExternalBuild_Add_Step(
    NAME "Update Cache"
    COMMAND ${CMAKE_COMMAND} -E touch "${CUSTOM_STAMP_FILE}"
)

# CMake requires include paths be valid at configure time
set(INCLUDE_DIR "${CACHE_DIR}/include")
file(GENERATE OUTPUT "${INCLUDE_DIR}/.eb" CONTENT "")

set(LIBRARY_DIR "${CACHE_DIR}/lib")
# boost_system
add_library(boost_system STATIC IMPORTED GLOBAL)
set_target_properties(
    boost_system
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIR}"
    IMPORTED_LOCATION_DEBUG "${LIBRARY_DIR}/libboost_system-mt-gd${CMAKE_STATIC_LIBRARY_SUFFIX}"
    IMPORTED_LOCATION_RELEASE "${LIBRARY_DIR}/libboost_system-mt${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
add_dependencies(boost_system build_boost)

# boost_filesystem
add_library(boost_filesystem STATIC IMPORTED GLOBAL)
set_target_properties(
    boost_filesystem
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIR}"
    IMPORTED_LOCATION_DEBUG "${LIBRARY_DIR}/libboost_filesystem-mt-gd${CMAKE_STATIC_LIBRARY_SUFFIX}"
    IMPORTED_LOCATION_RELEASE "${LIBRARY_DIR}/libboost_filesystem-mt${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
target_link_libraries(boost_filesystem LINK_INTERFACE_LIBRARIES boost_system)
add_dependencies(boost_filesystem build_boost)
