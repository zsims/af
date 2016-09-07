source_group("src" REGULAR_EXPRESSION "src/bs_backup_cli/.*")

set(SOURCES
    src/bs_backup_cli/bs_backup_cli_main.cpp
)

add_executable(bs_backup_cli ${SOURCES})
set_property(TARGET bs_backup_cli PROPERTY FOLDER "bs_cli")

target_include_directories(
    bs_backup_cli
    PRIVATE src
)

target_link_libraries(
    bs_backup_cli
    PRIVATE bslib
    PRIVATE boost_program_options
)

