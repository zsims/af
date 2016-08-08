source_group("src" REGULAR_EXPRESSION "src/.*")
source_group("src\\blob" REGULAR_EXPRESSION "src/blob/.*")
source_group("src\\file" REGULAR_EXPRESSION "src/file/.*")

add_executable(
    bslib_test
    src/bslib_test_main.cpp
    src/AddressTest.cpp
    src/ForestIntegrationTest.cpp
    src/blob/BlobInfoRepositoryIntegrationTest.cpp
    src/blob/DirectoryBlobStoreIntegrationTest.cpp
    src/file/FileObjectInfoRepositoryIntegrationTest.cpp
)

set_property(TARGET bslib_test PROPERTY FOLDER "bslib")

target_include_directories(
    bslib_test
    PRIVATE $<TARGET_PROPERTY:bslib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE ../src
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
