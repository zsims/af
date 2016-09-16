#include "bslib/file/fs/operations.hpp"

#include "bslib/unicode.hpp"
#include "bslib/file/fs/WindowsPath.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <windows.h>

#include <string>

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

WindowsPath GenerateUniqueTempPath(boost::system::error_code& ec) noexcept
{
	// Per https://msdn.microsoft.com/en-us/library/windows/desktop/aa364991(v=vs.85).aspx GUIDs are recommended for 
	// creating many temporary files to avoid clashes... so lets cut out the middle man ;)
	wchar_t buffer[MAX_PATH];
	if (!::GetTempPathW(MAX_PATH, buffer))
	{
		ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
		return WindowsPath();
	}
	WindowsPath result(buffer);
	result.AppendSegment(GenerateUuid());
	ec = boost::system::error_code();
	return result;
}

WindowsPath GenerateUniqueTempPath()
{
	boost::system::error_code ec;
	auto result = GenerateUniqueTempPath(ec);
	if (ec)
	{
		throw boost::system::system_error(ec, "Failed to generate unique temporary path");
	}
	return result;
}

bool IsDirectory(const WindowsPath& path, boost::system::error_code& ec) noexcept
{
	const auto wideString = path.ToExtendedWideString();
	const auto result = ::GetFileAttributesW(wideString.c_str());
	if (result == INVALID_FILE_ATTRIBUTES)
	{
		ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
		return false;
	}
	ec = boost::system::error_code();
	return (result & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
}

bool IsDirectory(const WindowsPath& path)
{
	boost::system::error_code ec;
	auto result = IsDirectory(path, ec);
	if (ec)
	{
		throw boost::system::system_error(ec, "Failed to determine if the given path is a directory");
	}
	return result;
}

bool CreateDirectorySexy(const WindowsPath& path, boost::system::error_code& ec) noexcept
{
	const auto wideString = path.ToExtendedWideString();
	// BOOL is a 32-bit int, so compare against false see https://msdn.microsoft.com/en-us/library/windows/desktop/bb773621(v=vs.85).aspx
	if (::CreateDirectoryW(wideString.c_str(), nullptr) != FALSE)
	{
		ec = boost::system::error_code();
		return true;
	}
	ec = boost::system::error_code(::GetLastError(), boost::system::system_category());
	return false;
}

bool CreateDirectorySexy(const WindowsPath& path)
{
	boost::system::error_code ec;
	auto result = CreateDirectorySexy(path, ec);
	if (ec)
	{
		throw boost::system::system_error(ec, "Failed to create directory");
	}
	return result;
}

bool CreateDirectories(const WindowsPath& path, boost::system::error_code& ec) noexcept
{
	if (IsDirectory(path, ec))
	{
		// didn't create it
		ec = boost::system::error_code(ERROR_ALREADY_EXISTS, boost::system::system_category());
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

bool CreateDirectories(const WindowsPath& path)
{
	boost::system::error_code ec;
	auto result = CreateDirectories(path, ec);
	if (ec)
	{
		throw boost::system::system_error(ec, "Failed to create drectories");
	}
	return result;
}

}
}
}
}