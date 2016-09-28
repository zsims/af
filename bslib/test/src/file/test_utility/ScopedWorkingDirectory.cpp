#include "file/test_utility/ScopedWorkingDirectory.hpp"

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <windows.h>

#include <memory>

namespace af {
namespace bslib {
namespace file {
namespace test_utility {

namespace {
void SetWorkingDirectory(const fs::NativePath& path)
{
	const auto wideString = UTF8ToWideString(path.ToExtendedString());
	const auto result = SetCurrentDirectoryW(wideString.c_str());
	if (!result)
	{
		throw boost::system::system_error(boost::system::error_code(::GetLastError(), boost::system::system_category()), "Failed to set working directory");
		return;
	}
}

fs::NativePath GetWorkingDirectory()
{
	const auto requiredBufferSize = GetCurrentDirectoryW(0, nullptr);
	if (requiredBufferSize == 0)
	{
		throw boost::system::system_error(boost::system::error_code(::GetLastError(), boost::system::system_category()), "Failed to get working directory");
		return fs::WindowsPath();
	}
	std::unique_ptr<wchar_t[]> buffer(new wchar_t[requiredBufferSize]);
	const auto writtenCharacterCount = GetCurrentDirectoryW(requiredBufferSize, &buffer[0]);
	if(requiredBufferSize == 0)
	{
		throw boost::system::system_error(boost::system::error_code(::GetLastError(), boost::system::system_category()), "Failed to get working directory");
		return fs::WindowsPath();
	}
	return fs::WindowsPath(WideToUTF8String(buffer.get()));
}
}

ScopedWorkingDirectory::ScopedWorkingDirectory(const fs::NativePath& path)
{
	_originalPath = GetWorkingDirectory();
	SetWorkingDirectory(path);
}

ScopedWorkingDirectory::~ScopedWorkingDirectory()
{
	try
	{
		SetWorkingDirectory(_originalPath);
	}
	catch (const boost::system::system_error&)
	{
		// NOM NOM NOM
	}
}

}
}
}
}
