cmake_minimum_required(VERSION 3.5)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake")

project(af LANGUAGES CXX C)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Modules
include(ExternalBuild)

if(MSVC)
    # Enable warnings as errors
    add_compile_options("/W3" "/WX")

    # x86 or x64, this makes it easier to deal with config
    # as CMAKE_VS_PLATFORM_NAME is Win32 on x86
    set(PLATFORM_NAME "x86")
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        set(PLATFORM_NAME "x64")
    endif()

    add_definitions("-DBOOST_CB_DISABLE_DEBUG")
    # Disable auto linking
    add_definitions("-DBOOST_ALL_NO_LIB")
endif()

# Setup a verify target + place for tests
set(TEST_RESULTS_DIR "${CMAKE_BINARY_DIR}/test_results")
file(MAKE_DIRECTORY "${TEST_RESULTS_DIR}")
add_custom_target(ALL_VERIFY)
# make sure everything builds when we run ALL_VERIFY
add_dependencies(ALL_VERIFY ALL_BUILD)

add_subdirectory(third_party)
add_subdirectory(ffs)
