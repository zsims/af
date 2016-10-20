set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")

BuildCache_Get(
    # -6 for our build config, add extra things here to control cache sharing
    KEY "boost-1.60.0-${PLATFORM_NAME}-6"
    CACHE_DIR_VAR CACHE_DIR
)

# Run build off this stamp (the cache)
set(CUSTOM_STAMP_FILE "${CACHE_DIR}/$<CONFIG>.success")

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
        --with-program_options
        --with-regex
        --with-log
        --with-thread
        --with-date_time
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

set(BOOST_INCLUDE_DIR "${INCLUDE_DIR}" PARENT_SCOPE)

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

# boost_program_options
add_library(boost_program_options STATIC IMPORTED GLOBAL)
set_target_properties(
    boost_program_options
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIR}"
    IMPORTED_LOCATION_DEBUG "${LIBRARY_DIR}/libboost_program_options-mt-gd${CMAKE_STATIC_LIBRARY_SUFFIX}"
    IMPORTED_LOCATION_RELEASE "${LIBRARY_DIR}/libboost_program_options-mt${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
target_link_libraries(boost_program_options LINK_INTERFACE_LIBRARIES boost_system)
add_dependencies(boost_program_options build_boost)

# boost_regex
add_library(boost_regex STATIC IMPORTED GLOBAL)
set_target_properties(
    boost_regex
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIR}"
    IMPORTED_LOCATION_DEBUG "${LIBRARY_DIR}/libboost_regex-mt-gd${CMAKE_STATIC_LIBRARY_SUFFIX}"
    IMPORTED_LOCATION_RELEASE "${LIBRARY_DIR}/libboost_regex-mt${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
add_dependencies(boost_regex build_boost)

# boost_date_time
add_library(boost_date_time STATIC IMPORTED GLOBAL)
set_target_properties(
    boost_date_time
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIR}"
    IMPORTED_LOCATION_DEBUG "${LIBRARY_DIR}/libboost_date_time-mt-gd${CMAKE_STATIC_LIBRARY_SUFFIX}"
    IMPORTED_LOCATION_RELEASE "${LIBRARY_DIR}/libboost_date_time-mt${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
target_link_libraries(boost_date_time LINK_INTERFACE_LIBRARIES boost_system)
add_dependencies(boost_date_time build_boost)

# boost_thread
add_library(boost_thread STATIC IMPORTED GLOBAL)
set_target_properties(
    boost_thread
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIR}"
    IMPORTED_LOCATION_DEBUG "${LIBRARY_DIR}/libboost_thread-mt-gd${CMAKE_STATIC_LIBRARY_SUFFIX}"
    IMPORTED_LOCATION_RELEASE "${LIBRARY_DIR}/libboost_thread-mt${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
target_link_libraries(boost_thread LINK_INTERFACE_LIBRARIES boost_system boost_date_time)
add_dependencies(boost_thread build_boost)

# boost_log
# TODO: this will need to be shared if this is used in multiple modules (dlls, etc) see http://www.boost.org/doc/libs/1_60_0/libs/log/doc/html/log/installation/config.html
add_library(boost_log STATIC IMPORTED GLOBAL)
set_target_properties(
    boost_log
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIR}"
    IMPORTED_LOCATION_DEBUG "${LIBRARY_DIR}/libboost_log-mt-gd${CMAKE_STATIC_LIBRARY_SUFFIX}"
    IMPORTED_LOCATION_RELEASE "${LIBRARY_DIR}/libboost_log-mt${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
target_link_libraries(boost_log LINK_INTERFACE_LIBRARIES boost_thread)
add_dependencies(boost_log build_boost)