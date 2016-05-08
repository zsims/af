source_group("src\\ffs" REGULAR_EXPRESSION "src/ffs/.*")
source_group("include" REGULAR_EXPRESSION "include/.*")

set(PUBLIC_INCLUDES
    include/ffs/FileSystem.hpp
)

set(SOURCES
    src/ffs/FileSystem.cpp
)

add_library(ffs ${SOURCES} ${PUBLIC_INCLUDES})
set_property(TARGET ffs PROPERTY FOLDER "ffs")

target_include_directories(
    ffs
    PUBLIC include
    PRIVATE src
)

add_subdirectory(test)
