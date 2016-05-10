source_group("src\\ffs" REGULAR_EXPRESSION "src/ffs/.*")
source_group("src\\ffs\\blob" REGULAR_EXPRESSION "src/ffs/blob/.*")
source_group("include\\ffs" REGULAR_EXPRESSION "include/ffs/.*")
source_group("include\\blob\\ffs" REGULAR_EXPRESSION "include/ffs/blob/.*")

set(PUBLIC_INCLUDES
    include/ffs/Address.hpp
    include/ffs/Forest.hpp
    include/ffs/blob/BlobInfo.hpp
    include/ffs/blob/BlobStore.hpp
    include/ffs/blob/DirectoryBlobStore.hpp
)

set(SOURCES
    src/ffs/Forest.cpp
    src/ffs/blob/BlobInfoRepository.cpp
    src/ffs/blob/BlobInfoRepository.hpp
    src/ffs/blob/BlobStore.cpp
    src/ffs/blob/DirectoryBlobStore.cpp
    src/ffs/blob/exceptions.hpp
)

add_library(ffs ${SOURCES} ${PUBLIC_INCLUDES})
set_property(TARGET ffs PROPERTY FOLDER "ffs")

target_include_directories(
    ffs
    PUBLIC include
    PRIVATE src
)

add_subdirectory(test)
