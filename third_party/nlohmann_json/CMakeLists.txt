# This is from https://github.com/nlohmann/json
source_group("src" REGULAR_EXPRESSION "src/.*")
set(SOURCES
    src/json.hpp
)
add_library(nlohmann_json INTERFACE)

target_include_directories(
    nlohmann_json
    INTERFACE src
)
