set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")

ExternalBuild_Add(
    build_boost
    SOURCE_DIR "${SOURCE_DIR}"
    BUILD_DIR_VAR BUILD_DIR
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
        link=static
        threading=multi
        toolset=msvc-14.0
        runtime-link=shared
        # Note the architecture for 64-bit is still x86, see http://www.boost.org/build/tutorial.html
        architecture=x86
        address-model=$<$<STREQUAL:x86,${PLATFORM_NAME}>:32>$<$<STREQUAL:x64,${PLATFORM_NAME}>:64>
        define=BOOST_CB_DISABLE_DEBUG
        variant=$<LOWER_CASE:$<CONFIG>>
    WORKING_DIRECTORY "${SOURCE_DIR}"
)

set(LIBRARY_DIR "${SOURCE_DIR}/stage/lib")

# boost_system
add_library(boost_system STATIC IMPORTED GLOBAL)
set_target_properties(
    boost_system
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${SOURCE_DIR}"
    IMPORTED_LOCATION_DEBUG "${LIBRARY_DIR}/libboost_system-mt-gd${CMAKE_STATIC_LIBRARY_SUFFIX}"
    IMPORTED_LOCATION_RELEASE "${LIBRARY_DIR}/libboost_system-mt${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
add_dependencies(boost_system build_boost)

# boost_filesystem
add_library(boost_filesystem STATIC IMPORTED GLOBAL)
set_target_properties(
    boost_filesystem
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${SOURCE_DIR}"
    IMPORTED_LOCATION_DEBUG "${LIBRARY_DIR}/libboost_filesystem-mt-gd${CMAKE_STATIC_LIBRARY_SUFFIX}"
    IMPORTED_LOCATION_RELEASE "${LIBRARY_DIR}/libboost_filesystem-mt${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
target_link_libraries(boost_filesystem LINK_INTERFACE_LIBRARIES boost_system)
add_dependencies(boost_filesystem build_boost)
