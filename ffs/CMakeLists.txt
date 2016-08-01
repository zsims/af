source_group("src\\ffs" REGULAR_EXPRESSION "src/ffs/.*")
source_group("src\\ffs\\blob" REGULAR_EXPRESSION "src/ffs/blob/.*")
source_group("src\\ffs\\object" REGULAR_EXPRESSION "src/ffs/object/.*")
source_group("src\\ffs\\sqlitepp" REGULAR_EXPRESSION "src/ffs/sqlitepp/.*")
source_group("include\\ffs" REGULAR_EXPRESSION "include/ffs/.*")
source_group("include\\blob\\ffs" REGULAR_EXPRESSION "include/ffs/blob/.*")
source_group("include\\object\\ffs" REGULAR_EXPRESSION "include/ffs/object/.*")

add_library(
    ffs
    include/ffs/Address.hpp
    include/ffs/ffs_fwd.hpp
    include/ffs/Forest.hpp
    include/ffs/UnitOfWork.hpp
    include/ffs/exceptions.hpp
    include/ffs/blob/BlobInfo.hpp
    include/ffs/blob/BlobStore.hpp
    include/ffs/blob/DirectoryBlobStore.hpp
    include/ffs/blob/NullBlobStore.hpp
    include/ffs/object/FileObjectInfo.hpp
    src/ffs/Address.cpp
    src/ffs/Forest.cpp
    src/ffs/ForestUnitOfWork.cpp
    src/ffs/ForestUnitOfWork.hpp
    src/ffs/blob/BlobInfoRepository.cpp
    src/ffs/blob/BlobInfoRepository.hpp
    src/ffs/blob/DirectoryBlobStore.cpp
    src/ffs/blob/exceptions.hpp
    src/ffs/object/FileObjectInfoRepository.cpp
    src/ffs/object/FileObjectInfoRepository.hpp
    src/ffs/object/exceptions.hpp
    src/ffs/sqlitepp/exceptions.hpp
    src/ffs/sqlitepp/handles.hpp
    src/ffs/sqlitepp/ScopedTransaction.hpp
    src/ffs/sqlitepp/ScopedTransaction.cpp
    src/ffs/sqlitepp/ScopedStatementReset.hpp
    src/ffs/sqlitepp/sqlitepp.cpp
    src/ffs/sqlitepp/sqlitepp.hpp
)
set_property(TARGET ffs PROPERTY FOLDER "ffs")

target_include_directories(
    ffs
    PUBLIC include
    PRIVATE src
    PUBLIC $<TARGET_PROPERTY:boost_filesystem,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:sqlite,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(
    ffs
    PRIVATE boost_filesystem
    PUBLIC sqlite
)

add_subdirectory(test)
