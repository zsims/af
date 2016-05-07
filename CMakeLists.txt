cmake_minimum_required(VERSION 3.5)

project(af)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(MSVC)
    # Enable warnings as errors
    add_compile_options("/W3" "/WX")
endif()

add_subdirectory(third_party)
add_subdirectory(cfs)
