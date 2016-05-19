# Module that can help with caching build data between builds, e.g. to improve ExternalProject build cycles.

if(WIN32)
    set(BUILD_CACHE_ROOT_DIR "$ENV{LOCALAPPDATA}/build_cache/${CMAKE_PROJECT_NAME}")
else()
    set(BUILD_CACHE_ROOT_DIR "$ENV{HOME}/build_cache/${CMAKE_PROJECT_NAME}")
endif()

file(TO_CMAKE_PATH "${BUILD_CACHE_ROOT_DIR}" BUILD_CACHE_ROOT_DIR)
file(MAKE_DIRECTORY "${BUILD_CACHE_ROOT_DIR}")
message(STATUS "Caching build data in ${BUILD_CACHE_ROOT_DIR}")

# BuildCache_Get
#   KEY             - Cache key, e.g. "boost"
#   CACHE_DIR_VAR  - Full path to the cache path
function(BuildCache_Get)
    set(ONE_VALUE_ARGS KEY CACHE_DIR_VAR)
    cmake_parse_arguments(ARGS "" "${ONE_VALUE_ARGS}" "" ${ARGN})
    set(${ARGS_CACHE_DIR_VAR} "${BUILD_CACHE_ROOT_DIR}/${ARGS_KEY}" PARENT_SCOPE)
endfunction()
