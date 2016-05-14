#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ffs/Address.hpp"

namespace af {
namespace ffs {
namespace test {

TEST(AddressTest, ConstructFromString)
{
	// Arrange
	const binary_address expectedBinaryAddress = {
		0x12, 0x34, 0xa1, 0x23, 0x45,
		0x12, 0x34, 0xb1, 0x23, 0x45,
		0x12, 0x34, 0xa1, 0x23, 0x45,
		0x12, 0x34, 0xb1, 0x23, 0x45
	};
	const auto stringAddress = "1234a123451234b123451234a123451234b12345";

	// Act
	const Address result(stringAddress);

	// Assert
	EXPECT_EQ(expectedBinaryAddress, result.ToBinary());
}

TEST(AddressTest, ConstructFromLongStringThrows)
{
	// Arrange
	const auto stringAddress = "1234a123451234b123451234a123451234b12345a"; // 41

	// Act
	// Assert
	// Note that the most vexing parse rule applies here, see http://stackoverflow.com/questions/6447596/googletest-does-not-accept-temporary-object-in-expect-throw
	EXPECT_THROW(Address{stringAddress}, InvalidAddressException);
}

TEST(AddressTest, ConstructFromShortStringThrows)
{
	// Arrange
	const auto stringAddress = "1234a123451234b123451234a123451234b1234"; // 39

	// Act
	// Assert
	EXPECT_THROW(Address{stringAddress}, InvalidAddressException);
}

TEST(AddressTest, ConstructFromBinary)
{
	// Arrange
	const binary_address binaryAddress = {
		0x12, 0x34, 0xa1, 0x23, 0x45,
		0x12, 0x34, 0xb1, 0x23, 0x45,
		0x12, 0x34, 0xa1, 0x23, 0x45,
		0x12, 0x34, 0xb1, 0x23, 0x45
	};

	// Act
	const Address result(binaryAddress);

	// Assert
	EXPECT_EQ("1234a123451234b123451234a123451234b12345", result.ToString());
}

TEST(AddressTest, CalculateFromContent)
{
	// Arrange
	const std::vector<uint8_t> content = {
		'h', 'e', 'l', 'l', 'o'
	};

	// Act
	const auto result = Address::CalculateFromContent(content);

	// Assert
	EXPECT_EQ("aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d", result.ToString());
}

}
}
}
