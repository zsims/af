source_group("src" REGULAR_EXPRESSION "src/.*")
source_group("src\\blob" REGULAR_EXPRESSION "src/blob/.*")
source_group("src\\object" REGULAR_EXPRESSION "src/object/.*")

add_executable(
    ffs_test
    src/ffs_test_main.cpp
    src/AddressTest.cpp
    src/ForestIntegrationTest.cpp
    src/blob/BlobInfoRepositoryIntegrationTest.cpp
    src/blob/DirectoryBlobStoreIntegrationTest.cpp
    src/object/ObjectInfoRepositoryIntegrationTest.cpp
)

set_property(TARGET ffs_test PROPERTY FOLDER "ffs")

target_include_directories(
    ffs_test
    PRIVATE $<TARGET_PROPERTY:ffs,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE ../src
)

target_link_libraries(
    ffs_test
    PRIVATE ffs
    PRIVATE googletest
)

add_custom_target(
    ffs_test_verify
    COMMAND "$<TARGET_FILE:ffs_test>" "--gtest_output=xml:${TEST_RESULTS_DIR}/ffs_test.xml"
)
set_property(TARGET ffs_test_verify PROPERTY FOLDER "ffs")
add_dependencies(ALL_VERIFY ffs_test_verify)
