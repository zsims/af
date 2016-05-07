add_executable(ffs_test src/ffs_test_main.cpp)

set_property(TARGET ffs_test PROPERTY FOLDER "ffs")

target_include_directories(
    ffs_test
    PRIVATE $<TARGET_PROPERTY:ffs,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(
    ffs_test
    PRIVATE googletest
)
