#pragma once

#include <boost/filesystem/path.hpp>

namespace af {
namespace bslib {
namespace file {

/**
 * Extension of boost::filesystem::path that ensures the path has a trailing separator
 */
class DirectoryPath : public boost::filesystem::path
{
public:
	explicit DirectoryPath(const boost::filesystem::path& path)
		: boost::filesystem::path(path)
	{
		remove_trailing_separator();
		*this += boost::filesystem::path::preferred_separator;
	}
};

}
}
}