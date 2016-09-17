#pragma once

#include <string>

namespace af {
namespace bslib {

typedef std::string UTF8String;

/**
 * Converts the given string to a wide string, replacing invalid characters as required (with U+FFFD, the unicode replacement character)
 * \remarks The encoding of wide string depends on the platform
 */
std::wstring UTF8ToWideString(const UTF8String& str);

/**
 * Converts the given wide string to a UTF-8 string, replacing invalid characters as required (with U+FFFD, the unicode replacement character)
 * \remarks The encoding of wide string depends on the platform
 */
UTF8String WideToUTF8String(const std::wstring& wideString);

}
}
