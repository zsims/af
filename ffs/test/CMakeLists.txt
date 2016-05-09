source_group("src" REGULAR_EXPRESSION "src/.*")
source_group("src\\repo" REGULAR_EXPRESSION "src/repo/.*")

add_executable(
    ffs_test
    src/ffs_test_main.cpp
    src/blob/BlobInfoRepositoryIntegrationTest.cpp
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
