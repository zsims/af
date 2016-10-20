source_group("src" REGULAR_EXPRESSION "src/bs_daemon_lib/.*")

# Separate main functionality into library so we can link these into a test
add_library(bs_daemon_lib STATIC
    src/bs_daemon_lib/FileBackupJob.cpp
    src/bs_daemon_lib/FileBackupJob.hpp
    src/bs_daemon_lib/HttpServer.cpp
    src/bs_daemon_lib/HttpServer.hpp
    src/bs_daemon_lib/log.hpp
    src/bs_daemon_lib/Job.hpp
    src/bs_daemon_lib/JobExecutor.cpp
    src/bs_daemon_lib/JobExecutor.hpp
)

set_property(TARGET bs_daemon_lib PROPERTY FOLDER "bs_daemon")

target_include_directories(
    bs_daemon_lib
    PUBLIC src
)

target_link_libraries(
    bs_daemon_lib
    PUBLIC bslib
    PUBLIC boost_filesystem
    PUBLIC boost_log
    PUBLIC webserver
)

# Simple exe target
source_group("src" REGULAR_EXPRESSION "src/bs_daemon/.*")
add_executable(bs_daemon src/bs_daemon/bs_daemon_main.cpp)
set_property(TARGET bs_daemon PROPERTY FOLDER "bs_daemon")
target_link_libraries(
    bs_daemon
    PRIVATE bs_daemon_lib
)

add_subdirectory(test)