#include "bslib/Uuid.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace af {
namespace bslib {
namespace test {

TEST(UuidTest, Construct_FromString)
{
	// Arrange
	// Act
	const Uuid uuid("C7F98911-6A3B-4543-B5D9-8DEAD56DE5B7");
	// Assert
	// No throw ;)
}

TEST(UuidTest, Construct_FromInvalidString)
{
	// Arrange
	// Act
	// Assert
	EXPECT_THROW(Uuid("your-mother"), UuidInvalidException);
	// No throw ;)
}

TEST(UuidTest, Construct_FromStringCaseInsensitive)
{
	// Arrange
	// Act
	const Uuid uuid("C7F98911-6A3B-4543-B5D9-8DEAD56DE5B7");
	const Uuid uuid2("c7f98911-6a3b-4543-b5d9-8dead56de5b7");
	// Assert
	EXPECT_EQ(uuid, uuid2);
}

TEST(UuidTest, Create_Success)
{
	// Arrange
	// Act
	const auto uuid = Uuid::Create();
	const auto uuid2 = Uuid::Create();
	// Assert
	EXPECT_NE(uuid, uuid2);
}

TEST(UuidTest, Equality_Success)
{
	// Arrange
	const Uuid uuid("C7F98911-6A3B-4543-B5D9-8DEAD56DE5B7");
	const Uuid uuid2("C7F98911-6A3B-4543-B5D9-8DEAD56DE5B7");
	// Act
	// Assert
	EXPECT_EQ(uuid, uuid2);
}

TEST(UuidTest, ToString_Success)
{
	// Arrange
	const auto stringValue = "C7F98911-6A3B-4543-B5D9-8DEAD56DE5B7";
	const auto lowerStringValue = "c7f98911-6a3b-4543-b5d9-8dead56de5b7";
	const Uuid uuid(stringValue);
	// Act
	const auto result = uuid.ToString();
	// Assert
	EXPECT_EQ(lowerStringValue, result);
}

TEST(UuidTest, Empty_ComparableSuccess)
{
	// Arrange
	const Uuid uuid("00000000-0000-0000-0000-000000000000");
	// Act
	// Assert
	EXPECT_EQ(Uuid::Empty, uuid);
}

TEST(UuidTest, ToArray_Success)
{
	// Arrange
	const auto uuid = Uuid::Create();
	// Act
	const auto result = uuid.ToArray();
	// Assert
	const Uuid andBack(&result[0], static_cast<int>(result.size()));
	EXPECT_EQ(uuid, andBack);
}

TEST(UuidTest, Copy_AreEquivalent)
{
	// Arrange
	const auto uuid = Uuid::Create();
	// Act
	const auto copy = uuid;
	// Assert
	EXPECT_EQ(uuid, copy);
}

}
}
}
