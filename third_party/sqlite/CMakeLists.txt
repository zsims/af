# Downloads a SQLite amalgamation, and builds it
# See https://www.sqlite.org/amalgamation.html

set(DOWNLOAD_URL "https://www.sqlite.org/2016/sqlite-amalgamation-3120200.zip")
set(DOWNLOAD_TO "${CMAKE_CURRENT_BINARY_DIR}/sqlite-amalgamation-3120200.zip")
set(SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/sqlite-amalgamation-3120200")
file(MAKE_DIRECTORY "${SOURCE_DIR}")

file(
    DOWNLOAD "${DOWNLOAD_URL}" "${DOWNLOAD_TO}"
    SHOW_PROGRESS
    TLS_VERIFY ON
    EXPECTED_HASH SHA1=22632bf0cfacedbeddde9f92695f71cab8d8c0a5
)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xzf "${DOWNLOAD_TO}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)


set(SOURCES
    ${SOURCE_DIR}/shell.c
    ${SOURCE_DIR}/sqlite3.c
    ${SOURCE_DIR}/sqlite3.h
    ${SOURCE_DIR}/sqlite3ext.h
)

add_library(sqlite STATIC ${SOURCES})
set_property(TARGET sqlite PROPERTY FOLDER "third_party")

target_include_directories(
    sqlite
    PUBLIC ${SOURCE_DIR}
)

# Name it "build_sqlite3" for consistency with other third party builds
set_property(TARGET sqlite PROPERTY PROJECT_LABEL "build_sqlite")
