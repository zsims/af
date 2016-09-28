#pragma once

#include "bslib/file/fs/path.hpp"

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace af {
namespace bslib {
namespace file {
namespace fs {

/**
* Generates a unique temporary path, e.g. C:\temp\<some guid>
* \remarks This will be shorter than GenerateUniqueTempPath() on platforms where the distinction between extended and normal paths exists
* \returns The full path to the unique temporary path
*/
NativePath GenerateShortUniqueTempPath(boost::system::error_code& ec) noexcept;
NativePath GenerateShortUniqueTempPath();

/**
 * Generates a unique temporary path that's beyond 260 characters, e.g. C:\temp\something so very long
 * \returns The full path to the unique temporary path
 */
NativePath GenerateUniqueTempPath(boost::system::error_code& ec) noexcept;
NativePath GenerateUniqueTempPath();

/**
 * Determines whether the given path exists and represents a directory
 * \returns true if the given path is a directory, false otherwise
 **/
bool IsDirectory(const NativePath& path, boost::system::error_code& ec) noexcept;
bool IsDirectory(const NativePath& path);

/**
 * Determines whether the given path exists and represents a regular file
 * \returns true if the given path is a regular file, false otherwise
 **/
bool IsRegularFile(const NativePath& path, boost::system::error_code& ec) noexcept;
bool IsRegularFile(const NativePath& path);

/**
 * Creates a directory at the given path.
 * \remarks This clashes with the Windows.h macro CreateDirectory
 * \returns true if a new directory was created, false otherwise
 */
bool CreateDirectorySexy(const NativePath& path, boost::system::error_code& ec) noexcept;
bool CreateDirectorySexy(const NativePath& path);

/**
 * Creates all directories up to and including the given path
 * \returns true if a new directory was created, false otherwise
 */
bool CreateDirectories(const NativePath& path, boost::system::error_code& ec) noexcept;
bool CreateDirectories(const NativePath& path);

/**
 * Computes a well formed absolute path from the given path segment that may be relative or absolute
 * \remarks This is not thread safe, as the "current directory" is a global concept
 * \returns A well formed path if ec is marked as success
 */
NativePath GetAbsolutePath(const std::wstring& path, boost::system::error_code& ec) noexcept;
NativePath GetAbsolutePath(const std::wstring& path);

}
}
}
}