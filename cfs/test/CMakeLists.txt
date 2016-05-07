add_executable(cfs_test src/cfs_test_main.cpp)

set_property(TARGET cfs_test PROPERTY FOLDER "cfs")

target_include_directories(
    cfs_test
    PRIVATE $<TARGET_PROPERTY:cfs,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(
    cfs_test
    PRIVATE googletest
)
