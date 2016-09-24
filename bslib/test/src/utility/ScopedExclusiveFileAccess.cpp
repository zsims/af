#pragma once

#include "utility/ScopedExclusiveFileAccess.hpp"

#if WIN32
#include <windows.h>
#endif

#include <stdexcept>

namespace af {
namespace bslib {
namespace test {
namespace utility {

ScopedExclusiveFileAccess::ScopedExclusiveFileAccess(const boost::filesystem::path& path)
	: _path(path)
{
	_windowsHandle = ::CreateFileA(path.string().c_str(), GENERIC_READ, 0 /* no share */, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
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
