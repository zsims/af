#include "bslib/file/fs/operations.hpp"

#include "bslib/file/fs/WindowsPath.hpp"
#include "bslib/unicode.hpp"

#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <windows.h>

#include <string>

/**
 * Note that all operations here should support unicode and extended paths.
 * Some boost operations support extended paths, in those cases it's preferrable to wrap boost with wide strings to avoid code page conversion
 */

namespace af {
namespace bslib {
namespace file {
namespace fs {

namespace {

std::string GenerateUuid()
{
	boost::uuids::random_generator generator;
	boost::uuids::uuid uuid = generator();
	return boost::uuids::to_string(uuid);
}

}

NativePath GenerateShortUniqueTempPath(boost::system::error_code& ec) noexcept
{
	// Per https://msdn.microsoft.com/en-us/library/windows/desktop/aa364991(v=vs.85).aspx GUIDs are recommended for 
	// creating many temporary files to avoid clashes... so lets cut out the middle man ;)
	wchar_t buffer[MAX_PATH];
	if (!::GetTempPathW(MAX_PATH, buffer))
	{
		ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
		return WindowsPath();
	}
	WindowsPath result(WideToUTF8String(buffer));
	result.AppendSegment(GenerateUuid());
	ec.clear();
	return result;
}

NativePath GenerateShortUniqueTempPath()
{
	boost::system::error_code ec;
	auto result = GenerateShortUniqueTempPath(ec);
	if (ec)
	{
		throw boost::system::system_error(ec, "Failed to generate unique temporary path");
	}
	return result;
}

NativePath GenerateUniqueTempPath(boost::system::error_code& ec) noexcept
{
	wchar_t buffer[MAX_PATH];
	if (!::GetTempPathW(MAX_PATH, buffer))
	{
		ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
		return WindowsPath();
	}
	WindowsPath result(WideToUTF8String(buffer));
	// put 150 characters into the path via creating a long directory name
	// note that files can still not be named > 255 characters
	const auto uniquePart = GenerateUuid();
	auto intermediate = result / (uniquePart + UTF8String(150, 'a'));
	CreateDirectorySexy(intermediate, ec);
	if (ec)
	{
		return WindowsPath();
	}
	return (intermediate / (uniquePart + UTF8String(150, 'b')));
}

WindowsPath GenerateUniqueTempPath()
{
	boost::system::error_code ec;
	auto result = GenerateUniqueTempPath(ec);
	if (ec)
	{
		throw boost::system::system_error(ec, "Failed to generate unique extended temporary path");
	}
	return result;
}

bool IsDirectory(const NativePath& path, boost::system::error_code& ec) noexcept
{
	const auto wideString = UTF8ToWideString(path.ToExtendedString());
	return boost::filesystem::is_directory(wideString, ec);
}

bool IsDirectory(const NativePath& path)
{
	const auto wideString = UTF8ToWideString(path.ToExtendedString());
	return boost::filesystem::is_directory(wideString);
}

bool IsRegularFile(const NativePath& path, boost::system::error_code& ec) noexcept
{
	const auto wideString = UTF8ToWideString(path.ToExtendedString());
	return boost::filesystem::is_regular_file(wideString, ec);
}

bool IsRegularFile(const NativePath& path)
{
	const auto wideString = UTF8ToWideString(path.ToExtendedString());
	return boost::filesystem::is_regular_file(wideString);
}

bool Exists(const NativePath& path, boost::system::error_code& ec) noexcept
{
	// This is safe with extended paths, and 99x easier than implementing it
	const auto wideString = UTF8ToWideString(path.ToExtendedString());
	return boost::filesystem::exists(wideString, ec);
}

bool Exists(const NativePath& path)
{
	const auto wideString = UTF8ToWideString(path.ToExtendedString());
	return boost::filesystem::exists(wideString);
}

bool CreateDirectorySexy(const NativePath& path, boost::system::error_code& ec) noexcept
{
	const auto wideString = UTF8ToWideString(path.ToExtendedString());
	// BOOL is a 32-bit int, so compare against false see https://msdn.microsoft.com/en-us/library/windows/desktop/bb773621(v=vs.85).aspx
	if (::CreateDirectoryW(wideString.c_str(), nullptr) != FALSE)
	{
		ec.clear();
		return true;
	}
	ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
	return false;
}

bool CreateDirectorySexy(const NativePath& path)
{
	boost::system::error_code ec;
	auto result = CreateDirectorySexy(path, ec);
	if (ec)
	{
		throw boost::system::system_error(ec, "Failed to create directory");
	}
	return result;
}

bool CreateDirectories(const NativePath& path, boost::system::error_code& ec) noexcept
{
	if (IsDirectory(path, ec))
	{
		// already exists
		return false;
	}

	const auto intermediatePaths = path.GetIntermediatePaths();
	for (const auto& intermediate : intermediatePaths)
	{
		if (!IsDirectory(intermediate, ec) && !CreateDirectorySexy(intermediate, ec))
		{
			return false;
		}
	}
	return true;
}

bool CreateDirectories(const NativePath& path)
{
	boost::system::error_code ec;
	auto result = CreateDirectories(path, ec);
	if (ec)
	{
		throw boost::system::system_error(ec, "Failed to create drectories");
	}
	return result;
}

void Remove(const NativePath& path, boost::system::error_code& ec) noexcept
{
	const auto wideString = UTF8ToWideString(path.ToExtendedString());
	boost::filesystem::remove(wideString, ec);
}

void Remove(const NativePath& path)
{
	const auto wideString = UTF8ToWideString(path.ToExtendedString());
	boost::filesystem::remove(wideString);
}

void RemoveAll(const NativePath& path, boost::system::error_code& ec) noexcept
{
	// This is safe with extended paths, and 99x easier than implementing it
	const auto wideString = UTF8ToWideString(path.ToExtendedString());
	boost::filesystem::remove_all(wideString, ec);
}

void RemoveAll(const NativePath& path)
{
	// This is safe with extended paths, and 99x easier than implementing it
	const auto wideString = UTF8ToWideString(path.ToExtendedString());
	boost::filesystem::remove_all(wideString);
}

NativePath GetAbsolutePath(const UTF8String& path, boost::system::error_code& ec) noexcept
{
	// Find out how big the buffer needs to be
	const auto wideString = UTF8ToWideString(path);
	const auto requiredBufferSize = GetFullPathNameW(wideString.c_str(), 0, nullptr, nullptr);
	if (requiredBufferSize == 0)
	{
		ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
		return WindowsPath();
	}

	// allocate a buffer without initializing the memory
	std::unique_ptr<wchar_t[]> buffer(new wchar_t[requiredBufferSize]);

	// Copy into the allocated buffer
	const auto writtenCharacterCount = GetFullPathNameW(wideString.c_str(), requiredBufferSize, buffer.get(), nullptr);
	if (writtenCharacterCount == 0)
	{
		ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
		return WindowsPath();
	}
	ec.clear();
	return WindowsPath(WideToUTF8String(buffer.get()));
}

NativePath GetAbsolutePath(const UTF8String& path)
{
	boost::system::error_code ec;
	auto result = GetAbsolutePath(path, ec);
	if (ec)
	{
		throw boost::system::system_error(ec, "Failed to get absolute path");
	}
	return result;
}

std::ifstream OpenFileRead(const NativePath& path, std::ios_base::openmode mode) noexcept
{
	// VC++ has a constructor that takes a wide string, note that this doesn't exist on other platforms
	std::ifstream file(UTF8ToWideString(path.ToExtendedString()), mode);
	return file;
}

std::ofstream OpenFileWrite(const NativePath& path, std::ios_base::openmode mode) noexcept
{
	// VC++ has a constructor that takes a wide string, note that this doesn't exist on other platforms
	std::ofstream file(UTF8ToWideString(path.ToExtendedString()), mode);
	return file;
}

}
}
}
}