source_group("src" REGULAR_EXPRESSION "src/.*")
set(SOURCES
    src/client_http.hpp
    src/server_http.hpp
)
add_library(webserver INTERFACE)

target_include_directories(
    webserver
    INTERFACE src
    INTERFACE ${BOOST_INCLUDE_DIR}
)

target_link_libraries(
    webserver
    INTERFACE boost_regex
)