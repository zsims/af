source_group(src REGULAR_EXPRESSION "src/.*")
source_group(include REGULAR_EXPRESSION "include/.*")

set(PUBLIC_INCLUDES
    include/cfs/FileSystem.hpp
)

set(SOURCES
    src/FileSystem.cpp
)

add_library(cfs ${SOURCES} ${PUBLIC_INCLUDES})
set_property(TARGET cfs PROPERTY FOLDER "cfs")

target_include_directories(
    cfs
    PUBLIC include
    PRIVATE src
)

add_subdirectory(test)
