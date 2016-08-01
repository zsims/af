source_group("src\\bslib" REGULAR_EXPRESSION "src/bslib/.*")
source_group("src\\bslib\\blob" REGULAR_EXPRESSION "src/bslib/blob/.*")
source_group("src\\bslib\\object" REGULAR_EXPRESSION "src/bslib/object/.*")
source_group("src\\bslib\\sqlitepp" REGULAR_EXPRESSION "src/bslib/sqlitepp/.*")
source_group("include\\bslib" REGULAR_EXPRESSION "include/bslib/.*")
source_group("include\\blob\\bslib" REGULAR_EXPRESSION "include/bslib/blob/.*")
source_group("include\\object\\bslib" REGULAR_EXPRESSION "include/bslib/object/.*")

add_library(
    bslib
    include/bslib/Address.hpp
    include/bslib/bslib_fwd.hpp
    include/bslib/Forest.hpp
    include/bslib/UnitOfWork.hpp
    include/bslib/exceptions.hpp
    include/bslib/blob/BlobInfo.hpp
    include/bslib/blob/BlobStore.hpp
    include/bslib/blob/DirectoryBlobStore.hpp
    include/bslib/blob/NullBlobStore.hpp
    include/bslib/object/FileObjectInfo.hpp
    src/bslib/Address.cpp
    src/bslib/Forest.cpp
    src/bslib/ForestUnitOfWork.cpp
    src/bslib/ForestUnitOfWork.hpp
    src/bslib/blob/BlobInfoRepository.cpp
    src/bslib/blob/BlobInfoRepository.hpp
    src/bslib/blob/DirectoryBlobStore.cpp
    src/bslib/blob/exceptions.hpp
    src/bslib/object/FileObjectInfoRepository.cpp
    src/bslib/object/FileObjectInfoRepository.hpp
    src/bslib/object/exceptions.hpp
    src/bslib/sqlitepp/exceptions.hpp
    src/bslib/sqlitepp/handles.hpp
    src/bslib/sqlitepp/ScopedTransaction.hpp
    src/bslib/sqlitepp/ScopedTransaction.cpp
    src/bslib/sqlitepp/ScopedStatementReset.hpp
    src/bslib/sqlitepp/sqlitepp.cpp
    src/bslib/sqlitepp/sqlitepp.hpp
)
set_property(TARGET bslib PROPERTY FOLDER "bslib")

target_include_directories(
    bslib
    PUBLIC include
    PRIVATE src
    PUBLIC $<TARGET_PROPERTY:boost_filesystem,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:sqlite,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(
    bslib
    PRIVATE boost_filesystem
    PUBLIC sqlite
)

add_subdirectory(test)
