#pragma once

#include "bslib/file/fs/WindowsPath.hpp"

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace af {
namespace bslib {
namespace file {
namespace fs {

/**
 * Generates a unique temporary path, e.g. C:\temp\<some guid>
 * \returns The full path to the unique temporary path
 */
WindowsPath GenerateUniqueTempPath(boost::system::error_code& ec) noexcept;
WindowsPath GenerateUniqueTempPath();

/**
 * Determines whether the given path exists and represents a directory
 * \returns true if the given path is a directory, false otherwise
 **/
bool IsDirectory(const WindowsPath& path, boost::system::error_code& ec) noexcept;
bool IsDirectory(const WindowsPath& path);

/**
 * Creates a directory at the given path.
 * \remarks This clashes with the Windows.h macro CreateDirectory
 * \returns true if a new directory was created, false otherwise
 */
bool CreateDirectorySexy(const WindowsPath& path, boost::system::error_code& ec) noexcept;
bool CreateDirectorySexy(const WindowsPath& path);

/**
 * Creates all directories up to and including the given path
 * \returns true if a new directory was created, false otherwise
 */
bool CreateDirectories(const WindowsPath& path, boost::system::error_code& ec) noexcept;
bool CreateDirectories(const WindowsPath& path);

}
}
}
}