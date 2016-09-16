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

/**
 * Sets the current working directory
 */
void SetWorkingDirectory(const WindowsPath& path, boost::system::error_code& ec) noexcept;
void SetWorkingDirectory(const WindowsPath& path);

/**
 * Gets the current working directory
 * \remarks TODO: Handle UNC paths as this may return \\server\xxx for example
 */
WindowsPath GetWorkingDirectory(boost::system::error_code& ec) noexcept;
WindowsPath GetWorkingDirectory();

/**
 * Computes a well formed absolute path from the given path segment that may be relative or absolute
 * \remarks This is not thread safe, as the "current directory" is a global concept
 * \returns A well formed path if ec is marked as success
 */
WindowsPath GetAbsolutePath(const std::wstring& path, boost::system::error_code& ec) noexcept;
WindowsPath GetAbsolutePath(const std::wstring& path);

}
}
}
}