#include "bslib/default_locations.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>


namespace af {
namespace bslib {
namespace test {

TEST(default_locationsIntegrationTest, GetDefaultBackupDatabasePath_Success)
{
	// Arrange
	// Act
	const auto path = GetDefaultBackupDatabasePath();
	// Assert
	EXPECT_FALSE(path.empty());
}


}
}
}
