#include "test_util/gtest_boost_filesystem_fix.hpp"

// googletest has trouble with "recursive containers" per https://github.com/google/googletest/issues/521
namespace boost {
namespace filesystem {
void PrintTo(const boost::filesystem::path& path, std::ostream* os)
{
	*os << path;
}
}
}
