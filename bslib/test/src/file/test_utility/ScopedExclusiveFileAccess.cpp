#include "file/test_utility/ScopedExclusiveFileAccess.hpp"

#include <windows.h>

#include <stdexcept>

namespace af {
namespace bslib {
namespace file {
namespace test_utility {

ScopedExclusiveFileAccess::ScopedExclusiveFileAccess(const file::fs::NativePath& path)
	: _path(path)
{
	const auto wideString = UTF8ToWideString(path.ToExtendedString());
	_windowsHandle = ::CreateFileW(wideString.c_str(), GENERIC_READ, 0 /* no share */, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (_windowsHandle == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error("Failed to open file exclusively");
	}
}

ScopedExclusiveFileAccess::~ScopedExclusiveFileAccess()
{
	if (_windowsHandle != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_windowsHandle);
	}
}

}
}
}
}
