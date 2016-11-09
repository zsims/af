source_group("src" REGULAR_EXPRESSION "src/.*")
source_group("src\\blob" REGULAR_EXPRESSION "src/blob/.*")
source_group("src\\file" REGULAR_EXPRESSION "src/file/.*")
source_group("src\\file\\fs" REGULAR_EXPRESSION "src/file/fs/.*")
source_group("src\\file\\test_utility" REGULAR_EXPRESSION "src/file/test_utility/.*")
source_group("src\\utility" REGULAR_EXPRESSION "src/utility/.*")

add_executable(
    bslib_test
    src/bslib_test_main.cpp
    src/EventManagerTest.cpp
    src/BackupIntegrationTest.cpp
    src/BackupDatabaseIntegrationTest.cpp
    src/blob/AddressIntegrationTest.cpp
    src/blob/BlobInfoRepositoryIntegrationTest.cpp
    src/blob/DirectoryBlobStoreIntegrationTest.cpp
    src/blob/MockBlobStore.hpp
    src/file/FileAdderIntegrationTest.cpp
    src/file/FileEventStreamRepositoryIntegrationTest.cpp
    src/file/FileRestorerIntegrationTest.cpp
    src/file/fs/WindowsPathIntegrationTest.cpp
    src/file/fs/operationsIntegrationTest.cpp
    src/file/test_utility/ScopedExclusiveFileAccess.cpp
    src/file/test_utility/ScopedExclusiveFileAccess.hpp
    src/file/test_utility/ScopedWorkingDirectory.cpp
    src/file/test_utility/ScopedWorkingDirectory.hpp
    src/ObjectPoolIntegrationTest.cpp
    src/unicodeIntegrationTest.cpp
    src/UuidTest.cpp
)

set_property(TARGET bslib_test PROPERTY FOLDER "bslib")

target_include_directories(
    bslib_test
    PRIVATE $<TARGET_PROPERTY:bslib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE ../src
    PRIVATE src
    PRIVATE $<TARGET_PROPERTY:sqlite,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(
    bslib_test
    PRIVATE bslib
    PRIVATE googletest
    PRIVATE boost_program_options
    PRIVATE bslib_test_util
)

add_custom_target(
    bslib_test_verify
    COMMAND "$<TARGET_FILE:bslib_test>" "--gtest_output=xml:${TEST_RESULTS_DIR}/bslib_test.xml" "--temp" "${CMAKE_CURRENT_BINARY_DIR}/test_temp"
)
set_property(TARGET bslib_test_verify PROPERTY FOLDER "bslib")
add_dependencies(ALL_VERIFY bslib_test_verify)
