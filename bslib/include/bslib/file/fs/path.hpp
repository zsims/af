#pragma once

#include "bslib/file/fs/WindowsPath.hpp"

#include <boost/filesystem/path.hpp>

namespace af {
namespace bslib {
namespace file {
namespace fs {

// Native path for the current platform
typedef WindowsPath NativePath;

/**
 * Returns a native path from the given boost path.
 * \remarks This correctly handles character encoding conversions
 */
NativePath NativeFromBoostPath(const boost::filesystem::path& path);

/**
 * Returns a boost path from the given native path.
 * \remarks This correctly handles character encoding conversions
 */
boost::filesystem::path BoostPathFromNative(const NativePath& path);

}
}
}
}
