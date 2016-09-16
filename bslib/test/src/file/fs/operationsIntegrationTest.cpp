﻿#include "bslib/file/fs/operations.hpp"

#include "bslib/file/exceptions.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fstream>
#include <string>

namespace af {
namespace bslib {
namespace file {
namespace fs {
namespace test {

TEST(operationsIntegrationTest, GenerateUniqueTempPath_Success)
{
	// Arrange
	// Act
	const auto first = GenerateUniqueTempPath();
	const auto second = GenerateUniqueTempPath();
	// Assert
	EXPECT_NE(first, second);
}

TEST(operationsIntegrationTest, IsDirectory_Success)
{
	// Arrange
	const auto first = GenerateUniqueTempPath();
	const auto parent = first.ParentPathCopy();

	// Act
	const auto isDirectory = IsDirectory(parent);

	// Assert
	EXPECT_TRUE(isDirectory);
}

TEST(operationsIntegrationTest, IsDirectory_NotExistSuccess)
{
	// Arrange
	const auto first = GenerateUniqueTempPath();

	// Act
	boost::system::error_code ec;
	const auto isDirectory = IsDirectory(first, ec);

	// Assert
	EXPECT_FALSE(isDirectory);
	EXPECT_TRUE(ec);
}

TEST(operationsIntegrationTest, CreateDirectorySexy_Success)
{
	// Arrange
	const auto unique = GenerateUniqueTempPath();

	// Act
	ASSERT_TRUE(CreateDirectorySexy(unique));

	// Assert
	const auto isDirectory = IsDirectory(unique);
	EXPECT_TRUE(isDirectory);
}

TEST(operationsIntegrationTest, CreateDirectorySexy_AlreadyExistingSuccess)
{
	// Arrange
	const auto unique = GenerateUniqueTempPath();
	CreateDirectorySexy(unique);

	// Act
	// Assert
	boost::system::error_code ec;
	EXPECT_FALSE(CreateDirectorySexy(unique, ec));
	EXPECT_TRUE(ec);
}

TEST(operationsIntegrationTest, CreateDirectories_Success)
{
	// Arrange
	const auto unique = GenerateUniqueTempPath();
	const auto full = unique / "and" / u8"我这样做对吗" / "deeper";

	// Act
	EXPECT_TRUE(CreateDirectories(full));

	// Assert
	EXPECT_TRUE(IsDirectory(full));
}

TEST(operationsIntegrationTest, CreateDirectories_ExtendedPathSuccess)
{
	// Arrange
	const auto unique = GenerateUniqueTempPath();
	const auto full = unique / "and" / UTF8String(150, 'a') / "deeper" / UTF8String(150, 'b') / u8"我";

	// Act
	ASSERT_TRUE(CreateDirectories(full));

	// Assert
	EXPECT_TRUE(IsDirectory(full));
}

TEST(operationsIntegrationTest, CreateDirectories_AlreadyExistsSuccess)
{
	// Arrange
	const auto unique = GenerateUniqueTempPath();
	ASSERT_TRUE(CreateDirectorySexy(unique));

	// Act
	// Assert
	boost::system::error_code ec;
	EXPECT_FALSE(CreateDirectories(unique, ec));
	EXPECT_TRUE(ec);
}

TEST(operationsIntegrationTest, CreateDirectories_FileExistsFails)
{
	// Arrange
	boost::system::error_code ec;
	const auto unique = GenerateUniqueTempPath();
	const auto widePath = unique.ToExtendedWideString();
	std::ofstream f(widePath.c_str(), std::ofstream::out | std::ofstream::binary);

	// Act
	// Assert
	EXPECT_FALSE(CreateDirectories(unique, ec));
	EXPECT_TRUE(ec);
}

}
}
}
}
}
