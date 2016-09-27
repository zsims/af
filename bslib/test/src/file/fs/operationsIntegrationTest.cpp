#include "bslib/file/fs/operations.hpp"

#include "bslib/file/exceptions.hpp"

#include <boost/scope_exit.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fstream>
#include <string>

namespace af {
namespace bslib {
namespace file {
namespace fs {
namespace test {

TEST(operationsIntegrationTest, GenerateUniqueTempExtendedPath_Success)
{
	// Arrange
	// Act
	const auto first = GenerateUniqueTempExtendedPath();
	const auto second = GenerateUniqueTempExtendedPath();
	// Assert
	EXPECT_NE(first, second);
}

TEST(operationsIntegrationTest, IsDirectory_Success)
{
	// Arrange
	const auto first = GenerateUniqueTempExtendedPath();
	const auto parent = first.ParentPathCopy();

	// Act
	const auto isDirectory = IsDirectory(parent);

	// Assert
	EXPECT_TRUE(isDirectory);
}

TEST(operationsIntegrationTest, IsDirectory_NotExistSuccess)
{
	// Arrange
	const auto first = GenerateUniqueTempExtendedPath();

	// Act
	boost::system::error_code ec;
	const auto isDirectory = IsDirectory(first, ec);

	// Assert
	EXPECT_FALSE(isDirectory);
	EXPECT_TRUE(ec);
}

TEST(operationsIntegrationTest, IsRegularFile_Success)
{
	// Arrange
	const auto first = GenerateUniqueTempExtendedPath();
	std::ofstream f(UTF8ToWideString(first.ToExtendedString()), std::ofstream::out | std::ofstream::binary);

	// Act
	const auto isFile = IsRegularFile(first);

	// Assert
	EXPECT_TRUE(isFile);
}

TEST(operationsIntegrationTest, IsRegularFile_NotExistSuccess)
{
	// Arrange
	const auto first = GenerateUniqueTempExtendedPath();

	// Act
	boost::system::error_code ec;
	const auto isDirectory = IsRegularFile(first, ec);

	// Assert
	EXPECT_FALSE(isDirectory);
	EXPECT_TRUE(ec);
}

TEST(operationsIntegrationTest, IsRegularFile_WithDirectoryFalse)
{
	// Arrange
	const auto first = GenerateUniqueTempExtendedPath();
	const auto parent = first.ParentPathCopy();

	// Act
	boost::system::error_code ec;
	const auto isFile = IsRegularFile(parent, ec);

	// Assert
	EXPECT_FALSE(isFile);
	EXPECT_FALSE(ec);
}

TEST(operationsIntegrationTest, CreateDirectorySexy_Success)
{
	// Arrange
	const auto unique = GenerateUniqueTempExtendedPath();

	// Act
	ASSERT_TRUE(CreateDirectorySexy(unique));

	// Assert
	const auto isDirectory = IsDirectory(unique);
	EXPECT_TRUE(isDirectory);
}

TEST(operationsIntegrationTest, CreateDirectorySexy_AlreadyExistingSuccess)
{
	// Arrange
	const auto unique = GenerateUniqueTempExtendedPath();
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
	const auto unique = GenerateUniqueTempExtendedPath();
	const auto full = unique / "and" / u8"我这样做对吗" / "deeper";

	// Act
	EXPECT_TRUE(CreateDirectories(full));

	// Assert
	EXPECT_TRUE(IsDirectory(full));
}

TEST(operationsIntegrationTest, CreateDirectories_ExtendedPathSuccess)
{
	// Arrange
	const auto unique = GenerateUniqueTempExtendedPath();
	const auto full = unique / "and" / UTF8String(150, 'a') / "deeper" / UTF8String(150, 'b') / u8"我";

	// Act
	ASSERT_TRUE(CreateDirectories(full));

	// Assert
	EXPECT_TRUE(IsDirectory(full));
}

TEST(operationsIntegrationTest, CreateDirectories_AlreadyExistsSuccess)
{
	// Arrange
	const auto unique = GenerateUniqueTempExtendedPath();
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
	const auto unique = GenerateUniqueTempExtendedPath();
	const auto widePath = UTF8ToWideString(unique.ToExtendedString());
	std::ofstream f(widePath.c_str(), std::ofstream::out | std::ofstream::binary);

	// Act
	// Assert
	EXPECT_FALSE(CreateDirectories(unique, ec));
	EXPECT_TRUE(ec);
}

TEST(operationsIntegrationTest, GetAbsolutePath_Success)
{
	// Arrange
	const auto originalWorkingDir = GetWorkingDirectory();
	BOOST_SCOPE_EXIT(&originalWorkingDir)
	{
		SetWorkingDirectory(originalWorkingDir);
	} BOOST_SCOPE_EXIT_END

	const auto expected = GenerateUniqueTempExtendedPath();
	const auto name = expected.GetFilename();
	const auto relativePath = UTF8ToWideString(name + R"(\..\)" + name );
	SetWorkingDirectory(expected / "..");

	// Act
	const auto actual = GetAbsolutePath(relativePath);

	// Assert
	EXPECT_EQ(expected, actual);
}

TEST(operationsIntegrationTest, GetAbsolutePath_RootSuccess)
{
	// Arrange
	const auto originalWorkingDir = GetWorkingDirectory();
	BOOST_SCOPE_EXIT(&originalWorkingDir)
	{
		SetWorkingDirectory(originalWorkingDir);
	} BOOST_SCOPE_EXIT_END

	const auto unique = GenerateUniqueTempExtendedPath();
	const auto root = unique.GetIntermediatePaths()[0];
	SetWorkingDirectory(root);

	// Act
	auto actual = GetAbsolutePath(L".");
	actual.EnsureTrailingSlash();

	// Assert
	EXPECT_EQ(root, actual);
}

}
}
}
}
}
