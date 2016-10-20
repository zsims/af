# Builds a local copy of the SQLite amalgamation. See https://www.sqlite.org/amalgamation.html

source_group("src" REGULAR_EXPRESSION "src/.*")
set(SOURCES
    src/sqlite3.c
    src/sqlite3.h
    src/sqlite3ext.h
)
add_library(sqlite STATIC ${SOURCES})
set_property(TARGET sqlite PROPERTY FOLDER "third_party")

# Enable SQLite foreign key constraints by default per https://www.sqlite.org/compile.html
add_definitions(-DSQLITE_DEFAULT_FOREIGN_KEYS=1)

# Don't sniff the platform using the now deprecated GetVersionEx()
# SQLite will assume this is Windows NT (Windows 7+ etc) without this check
add_definitions(-DSQLITE_WIN32_GETVERSIONEX=0)

target_include_directories(
    sqlite
    PUBLIC src
)

# Name it "build_sqlite3" for consistency with other third party builds
set_property(TARGET sqlite PROPERTY PROJECT_LABEL "build_sqlite")
