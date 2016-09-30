source_group("src" REGULAR_EXPRESSION "src/bs_restore_cli/.*")

set(SOURCES
    src/bs_restore_cli/bs_restore_cli_main.cpp
)

add_executable(bs_restore_cli ${SOURCES})
set_property(TARGET bs_restore_cli PROPERTY FOLDER "bs_cli")

target_include_directories(
    bs_restore_cli
    PRIVATE src
)

target_link_libraries(
    bs_restore_cli
    PRIVATE bslib
    PRIVATE boost_program_options
)

