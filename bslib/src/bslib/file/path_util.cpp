#include "bslib/file/path_util.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/path.hpp>

namespace af {
namespace bslib {
namespace file {

boost::filesystem::path& EnsureTrailingSlash(boost::filesystem::path& path)
{
	path.remove_trailing_separator();
	path += boost::filesystem::path::preferred_separator;
	return path;
}

boost::filesystem::path EnsureTrailingSlashCopy(const boost::filesystem::path& path)
{
	auto copy = path;
	copy.remove_trailing_separator();
	copy += boost::filesystem::path::preferred_separator;
	return copy;
}

boost::filesystem::path CombineFullPaths(const boost::filesystem::path& root, const boost::filesystem::path& other)
{
	// See http://www.boost.org/doc/libs/1_61_0/libs/filesystem/doc/portability_guide.htm for details on what should be done here
	// TODO: Handle other cases/platforms
	const auto escapedPath = boost::replace_all_copy(other.string(), ":", "");
	return root / escapedPath;
}

}
}
}