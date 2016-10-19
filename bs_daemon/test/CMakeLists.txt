source_group("src" REGULAR_EXPRESSION "src/.*")

add_executable(
    bs_daemon_test
    src/bs_daemon_test_main.cpp
    src/JobExecutorIntegrationTest.cpp
)

set_property(TARGET bs_daemon_test PROPERTY FOLDER "bs_daemon")

target_include_directories(
    bs_daemon_test
    PRIVATE src
    PRIVATE ../src
)

target_link_libraries(
    bs_daemon_test
    PRIVATE bs_daemon_lib
    PRIVATE googletest
    PRIVATE boost_program_options
    PRIVATE bslib_test_util
)

add_custom_target(
    bs_daemon_test_verify
    COMMAND "$<TARGET_FILE:bs_daemon_test>" "--gtest_output=xml:${TEST_RESULTS_DIR}/bs_daemon_test.xml" "--temp" "${CMAKE_CURRENT_BINARY_DIR}/test_temp"
)
set_property(TARGET bs_daemon_test_verify PROPERTY FOLDER "bs_daemon")
add_dependencies(ALL_VERIFY bs_daemon_test_verify)
