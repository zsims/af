cmake_minimum_required(VERSION 3.5)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake")

project(af LANGUAGES CXX C)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Modules
include(ExternalBuild)
include(BuildCache)

if(MSVC)
    # Enable warnings as errors
    add_compile_options("/W3" "/WX")

    # Disable specific warnings in VS
    # C4503 - Disable decorated name length exceeded, https://msdn.microsoft.com/en-us/library/074af4b6.aspx
    add_definitions("/wd4503")

    # Slim down windows.h includes by default
    add_definitions(-DWIN32_LEAN_AND_MEAN)

    # Use the Windows 7 SDK per https://msdn.microsoft.com/en-us/library/6sehtctf.aspx
    add_definitions(-D_WIN32_WINNT=0x0601)

    # Per https://github.com/google/googletest/issues/799 Google Test doesn't correctly detect C++11 support on VS2015
    # Forcing it ideal, as there's some things missing
    add_definitions(-DGTEST_LANG_CXX11)
    
    # x86 or x64, this makes it easier to deal with config
    # as CMAKE_VS_PLATFORM_NAME is Win32 on x86
    set(PLATFORM_NAME "x86")
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        set(PLATFORM_NAME "x64")
    endif()

    add_definitions(-DBOOST_CB_DISABLE_DEBUG)
    # Disable auto linking
    add_definitions(-DBOOST_ALL_NO_LIB)

    # Ensure all Win32 APIs take wide characters, avoiding accidentally passing non-wide ANSI strings
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# Setup a verify target + place for tests
set(TEST_RESULTS_DIR "${CMAKE_BINARY_DIR}/test_results")
file(MAKE_DIRECTORY "${TEST_RESULTS_DIR}")
add_custom_target(ALL_VERIFY)
# make sure everything builds when we run ALL_VERIFY
add_dependencies(ALL_VERIFY ALL_BUILD)

add_subdirectory(third_party)
add_subdirectory(bslib)
add_subdirectory(bs_cli)
add_subdirectory(bs_daemon)
