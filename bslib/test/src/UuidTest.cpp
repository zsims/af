#include "bslib/Uuid.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/algorithm/string.hpp>

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
	const auto stringValue = boost::algorithm::to_lower_copy(std::string("C7F98911-6A3B-4543-B5D9-8DEAD56DE5B7"));
	const Uuid uuid(stringValue);
	// Act
	const auto result = boost::algorithm::to_lower_copy(uuid.ToString());
	// Assert
	EXPECT_EQ(stringValue, result);
}

}
}
}
