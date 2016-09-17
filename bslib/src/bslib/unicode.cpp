#include "bslib/unicode.hpp"

#include <string>

#include <windows.h>

namespace af {
namespace bslib {

std::wstring UTF8ToWideString(const UTF8String& str)
{
	if (str.empty())
	{
		return std::wstring();
	}

	const auto inputSize = static_cast<int>(str.size());
	const auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &str[0], inputSize, nullptr, 0);
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], inputSize, &result[0], sizeNeeded);
	return result;
}

UTF8String WideToUTF8String(const std::wstring& wideString)
{
	if (wideString.empty())
	{
		return UTF8String();
	}
	const auto inputSize = static_cast<int>(wideString.size());
	const auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wideString[0], inputSize, nullptr, 0, nullptr, nullptr);
	UTF8String result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wideString[0], inputSize, &result[0], sizeNeeded, nullptr, nullptr);
	return result;
}

}
}
