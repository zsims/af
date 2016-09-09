#pragma once

#include <boost/filesystem/path.hpp>

namespace af {
namespace bslib {
namespace file {

/**
 * Ensures that the given path has a trailing separator
 */
boost::filesystem::path& EnsureTrailingSlash(boost::filesystem::path& path);
boost::filesystem::path EnsureTrailingSlashCopy(const boost::filesystem::path& path);

/**
 * Combines the two given full paths, escaping characters in `other` so that it is a subpath of `root`.
 * For example CombineFullPaths("C:\foo\bar", "D:\what\is\this") = "C:\foo\bar\D\what\is\this"
 */
boost::filesystem::path CombineFullPaths(const boost::filesystem::path& root, const boost::filesystem::path& other);

}
}
}