source_group("src" REGULAR_EXPRESSION "src/.*")
source_group("src\\mocks" REGULAR_EXPRESSION "src/bslib_test_util/mocks/.*")

add_library(bslib_test_util STATIC
    src/bslib_test_util/TestBackup.cpp
    src/bslib_test_util/TestBackup.hpp
    src/bslib_test_util/TestBase.cpp
    src/bslib_test_util/TestBase.hpp
    src/bslib_test_util/TestEnvironment.cpp
    src/bslib_test_util/TestEnvironment.hpp
    src/bslib_test_util/gtest_boost_filesystem_fix.cpp
    src/bslib_test_util/gtest_boost_filesystem_fix.hpp
    src/bslib_test_util/matchers.cpp
    src/bslib_test_util/matchers.hpp
    src/bslib_test_util/mocks/MockBackup.hpp
    src/bslib_test_util/mocks/MockUnitOfWork.hpp
)

set_property(TARGET bslib_test_util PROPERTY FOLDER "bslib")

target_include_directories(
    bslib_test_util
    PUBLIC src
    PUBLIC $<TARGET_PROPERTY:sqlite,INTERFACE_INCLUDE_DIRECTORIES>
    # Include all bslib paths, including private ones so we can use "internal" classes
    PUBLIC $<TARGET_PROPERTY:bslib,INCLUDE_DIRECTORIES>
)

target_link_libraries(
    bslib_test_util
    PUBLIC bslib
    PUBLIC googletest
)

