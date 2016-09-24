#include "bslib/unicode.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <array>
#include <vector>

namespace af {
namespace bslib {
namespace test {

TEST(unicodeIntegrationTest, WideToUTF8String_Success)
{
	// Arrange
	const auto input = L"你好，世界";
	const auto expected = u8"你好，世界";

	// Act
	const auto actual = WideToUTF8String(input);

	// Assert
	EXPECT_EQ(expected, actual);
}

TEST(unicodeIntegrationTest, WideToUTF8String_EmptySuccess)
{
	// Arrange
	const auto input = L"";
	const auto expected = u8"";

	// Act
	const auto actual = WideToUTF8String(input);

	// Assert
	EXPECT_EQ(expected, actual);
}

TEST(unicodeIntegrationTest, WideToUtf8_Success)
{
	// Arrange
	const auto input = u8"你好，世界";
	const auto expected = L"你好，世界";

	// Act
	const auto actual = UTF8ToWideString(input);

	// Assert
	EXPECT_EQ(expected, actual);
}

TEST(unicodeIntegrationTest, WideToUtf8_EmptySuccess)
{
	// Arrange
	const auto input = u8"";
	const auto expected = L"";

	// Act
	const auto actual = UTF8ToWideString(input);

	// Assert
	EXPECT_EQ(expected, actual);
}

TEST(unicodeIntegrationTest, WideToUtf8_ReplacesInvalidCharacters)
{
	// Arrange
	// 0xA0 0xA1 is not a valid sequence in UTF-8
	const auto input = "hello\xa0\xa1zac";
	const auto expected = L"hello\xFFFD\xFFFDzac";

	// Act
	const auto actual = UTF8ToWideString(input);

	// Assert
	EXPECT_EQ(expected, actual);
}

}
}
}
