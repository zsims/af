source_group("src" REGULAR_EXPRESSION "src/.*")

add_library(test_util STATIC
    src/test_util/TestEnvironment.cpp
    src/test_util/TestEnvironment.hpp
)

target_include_directories(
    test_util
    PUBLIC src
)

target_link_libraries(
    test_util
    PRIVATE bslib
    PRIVATE googletest
)

