#pragma once

#include "bslib/file/fs/path.hpp"

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <fstream>

namespace af {
namespace bslib {
namespace file {
namespace fs {

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
 * Determines whether the given path exists
 * \returns true if the given path exists, false otherwise
 **/
bool Exists(const NativePath& path, boost::system::error_code& ec) noexcept;
bool Exists(const NativePath& path);

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
 * Deletes the given file/directory
 */
void Remove(const NativePath& path, boost::system::error_code& ec) noexcept;
void Remove(const NativePath& path);

/**
 * Deletes the given file/directories
 */
void RemoveAll(const NativePath& path, boost::system::error_code& ec) noexcept;
void RemoveAll(const NativePath& path);

/**
 * Computes a well formed absolute path from the given path segment that may be relative or absolute
 * \remarks This is not thread safe, as the "current directory" is a global concept
 * \returns A well formed path if ec is marked as success
 */
NativePath GetAbsolutePath(const UTF8String& path, boost::system::error_code& ec) noexcept;
NativePath GetAbsolutePath(const UTF8String& path);

/**
 * Opens the file at the given path for reading
 */
std::ifstream OpenFileRead(const NativePath& path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::binary) noexcept;

/**
 * Opens the file at the given path for writing
 */
std::ofstream OpenFileWrite(const NativePath& path, std::ios_base::openmode mode = std::ios_base::out | std::ios_base::binary) noexcept;

}
}
}
}