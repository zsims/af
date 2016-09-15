source_group("src" REGULAR_EXPRESSION "src/.*")
source_group("src\\blob" REGULAR_EXPRESSION "src/blob/.*")
source_group("src\\file" REGULAR_EXPRESSION "src/file/.*")
source_group("src\\file\\fs" REGULAR_EXPRESSION "src/file/fs/.*")
source_group("src\\utility" REGULAR_EXPRESSION "src/utility/.*")

add_executable(
    bslib_test
    src/bslib_test_main.cpp
    src/EventManagerTest.cpp
    src/ForestIntegrationTest.cpp
    src/blob/AddressIntegrationTest.cpp
    src/blob/BlobInfoRepositoryIntegrationTest.cpp
    src/blob/DirectoryBlobStoreIntegrationTest.cpp
    src/file/FileAdderEsIntegrationTest.cpp
    src/file/FileEventStreamRepositoryIntegrationTest.cpp
    src/file/FileRestorerEsIntegrationTest.cpp
    src/file/fs/WindowsPathIntegrationTest.cpp
    src/unicodeIntegrationTest.cpp
    src/utility/gtest_boost_filesystem_fix.cpp
    src/utility/gtest_boost_filesystem_fix.hpp
    src/utility/matchers.cpp
    src/utility/matchers.hpp
    src/utility/ScopedExclusiveFileAccess.cpp
    src/utility/ScopedExclusiveFileAccess.hpp
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
)

add_custom_target(
    bslib_test_verify
    COMMAND "$<TARGET_FILE:bslib_test>" "--gtest_output=xml:${TEST_RESULTS_DIR}/bslib_test.xml"
)
set_property(TARGET bslib_test_verify PROPERTY FOLDER "bslib")
add_dependencies(ALL_VERIFY bslib_test_verify)
