source_group("src" REGULAR_EXPRESSION "src/.*")
source_group("src\\mocks" REGULAR_EXPRESSION "src/mocks/.*")

add_library(test_util STATIC
    src/test_util/TestBackup.cpp
    src/test_util/TestBackup.hpp
    src/test_util/TestBase.cpp
    src/test_util/TestBase.hpp
    src/test_util/TestEnvironment.cpp
    src/test_util/TestEnvironment.hpp
    src/test_util/gtest_boost_filesystem_fix.cpp
    src/test_util/gtest_boost_filesystem_fix.hpp
    src/test_util/matchers.cpp
    src/test_util/matchers.hpp
    src/test_util/mocks/MockBackup.hpp
    src/test_util/mocks/MockUnitOfWork.hpp
)

target_include_directories(
    test_util
    PUBLIC src
    PRIVATE $<TARGET_PROPERTY:sqlite,INTERFACE_INCLUDE_DIRECTORIES>
    # Include all bslib paths, including private ones so we can use "internal" classes
    PRIVATE $<TARGET_PROPERTY:bslib,INCLUDE_DIRECTORIES>
)

target_link_libraries(
    test_util
    PRIVATE bslib
    PRIVATE googletest
)

