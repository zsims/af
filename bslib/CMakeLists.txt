source_group("src\\bslib" REGULAR_EXPRESSION "src/bslib/.*")
source_group("src\\bslib\\blob" REGULAR_EXPRESSION "src/bslib/blob/.*")
source_group("src\\bslib\\file" REGULAR_EXPRESSION "src/bslib/file/.*")
source_group("src\\bslib\\file\\fs" REGULAR_EXPRESSION "src/bslib/file/fs/.*")
source_group("src\\bslib\\sqlitepp" REGULAR_EXPRESSION "src/bslib/sqlitepp/.*")
source_group("include\\bslib" REGULAR_EXPRESSION "include/bslib/.*")
source_group("include\\file" REGULAR_EXPRESSION "include/bslib/file/.*")

add_library(
    bslib
    include/bslib/exceptions.hpp
    include/bslib/Backup.hpp
    include/bslib/blob/Address.hpp
    include/bslib/blob/BlobStore.hpp
    include/bslib/blob/BlobStoreManager.hpp
    include/bslib/blob/DirectoryBlobStore.hpp
    include/bslib/blob/NullBlobStore.hpp
    include/bslib/default_locations.hpp
    include/bslib/EventManager.hpp
    include/bslib/file/exceptions.hpp
    include/bslib/file/FileAdder.hpp
    include/bslib/file/FileBackupRunEvent.hpp
    include/bslib/file/FileBackupRunReader.hpp
    include/bslib/file/FileBackupRunRecorder.hpp
    include/bslib/file/FileBackupRunSearchCriteria.hpp
    include/bslib/file/FileEvent.hpp
    include/bslib/file/FileEventSearchCriteria.hpp
    include/bslib/file/FileFinder.hpp
    include/bslib/file/FilePathSearchCriteria.hpp
    include/bslib/file/FileRestoreEvent.hpp
    include/bslib/file/FileRestorer.hpp
    include/bslib/file/FileType.hpp
    include/bslib/file/fs/path.hpp
    include/bslib/file/fs/WindowsPath.hpp
    include/bslib/file/VirtualFile.hpp
    include/bslib/file/VirtualFileBrowser.hpp
    include/bslib/UnitOfWork.hpp
    include/bslib/unicode.hpp
    include/bslib/Uuid.hpp
    src/bslib/Backup.cpp
    src/bslib/BackupDatabase.hpp
    src/bslib/blob/Address.cpp
    src/bslib/blob/BlobInfo.hpp
    src/bslib/blob/BlobInfoRepository.cpp
    src/bslib/blob/BlobInfoRepository.hpp
    src/bslib/blob/BlobStoreManager.cpp
    src/bslib/blob/DirectoryBlobStore.cpp
    src/bslib/blob/exceptions.hpp
    src/bslib/blob/NullBlobStore.cpp
    src/bslib/BackupDatabase.cpp
    src/bslib/BackupDatabaseConnection.hpp
    src/bslib/BackupDatabaseUnitOfWork.cpp
    src/bslib/BackupDatabaseUnitOfWork.hpp
    src/bslib/default_locations.cpp
    src/bslib/log.hpp
    src/bslib/file/FileAdder.cpp
    src/bslib/file/FileBackupRunEvent.cpp
    src/bslib/file/FileBackupRunEventStreamRepository.cpp
    src/bslib/file/FileBackupRunEventStreamRepository.hpp
    src/bslib/file/FileBackupRunReader.cpp
    src/bslib/file/FileBackupRunRecorder.cpp
    src/bslib/file/FileEvent.cpp
    src/bslib/file/FileEventStreamRepository.cpp
    src/bslib/file/FileEventStreamRepository.hpp
    src/bslib/file/FileFinder.cpp
    src/bslib/file/FilePathRepository.cpp
    src/bslib/file/FilePathRepository.hpp
    src/bslib/file/FileRestorer.cpp
    src/bslib/file/fs/operations.cpp
    src/bslib/file/fs/operations.hpp
    src/bslib/file/fs/path.cpp
    src/bslib/file/fs/WindowsPath.cpp
    src/bslib/file/VirtualFileBrowser.cpp
    src/bslib/ObjectPool.hpp
    src/bslib/sqlitepp/exceptions.hpp
    src/bslib/sqlitepp/handles.hpp
    src/bslib/sqlitepp/ScopedStatementReset.hpp
    src/bslib/sqlitepp/ScopedTransaction.cpp
    src/bslib/sqlitepp/ScopedTransaction.hpp
    src/bslib/sqlitepp/sqlitepp.cpp
    src/bslib/sqlitepp/sqlitepp.hpp
    src/bslib/unicode.cpp
    src/bslib/Uuid.cpp
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
    PUBLIC boost_log
    PUBLIC boost_date_time
    PUBLIC sqlite
    PUBLIC nlohmann_json
    PUBLIC shlwapi.lib
)

add_subdirectory(test)
add_subdirectory(test_util)
