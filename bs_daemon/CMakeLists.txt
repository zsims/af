source_group("src" REGULAR_EXPRESSION "src/bs_daemon/.*")

set(SOURCES
    src/bs_daemon/bs_daemon_main.cpp
)

add_executable(bs_daemon ${SOURCES})
set_property(TARGET bs_daemon PROPERTY FOLDER "bs_daemon")

target_include_directories(
    bs_daemon
    PRIVATE src
)

target_link_libraries(
    bs_daemon
    PRIVATE bslib
    PRIVATE boost_filesystem
    PRIVATE webserver
)
