#include "bslib/file/fs/operations.hpp"
#include "bslib/file/exceptions.hpp"
#include "file/test_utility/ScopedWorkingDirectory.hpp"

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

TEST(operationsIntegrationTest, IsRegularFile_Success)
{
	// Arrange
	const auto first = GenerateUniqueTempPath();
	std::ofstream f(UTF8ToWideString(first.ToExtendedString()), std::ofstream::out | std::ofstream::binary);

	// Act
	const auto isFile = IsRegularFile(first);

	// Assert
	EXPECT_TRUE(isFile);
}

TEST(operationsIntegrationTest, IsRegularFile_NotExistSuccess)
{
	// Arrange
	const auto first = GenerateUniqueTempPath();

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
	const auto first = GenerateUniqueTempPath();
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
	EXPECT_FALSE(ec);
}

TEST(operationsIntegrationTest, CreateDirectories_FileExistsFails)
{
	// Arrange
	boost::system::error_code ec;
	const auto unique = GenerateUniqueTempPath();
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
	const auto expected = GenerateShortUniqueTempPath();
	const auto name = expected.GetFilename();
	const auto relativePath = name + R"(\..\)" + name;
	test_utility::ScopedWorkingDirectory workingDirectory(expected / "..");

	// Act
	const auto actual = GetAbsolutePath(relativePath);

	// Assert
	EXPECT_EQ(expected, actual);
}

TEST(operationsIntegrationTest, GetAbsolutePath_RootSuccess)
{
	// Arrange
	const auto unique = GenerateShortUniqueTempPath();
	const auto root = unique.GetIntermediatePaths()[0];
	test_utility::ScopedWorkingDirectory workingDirectory(root);

	// Act
	auto actual = GetAbsolutePath(".");
	actual.EnsureTrailingSlash();

	// Assert
	EXPECT_EQ(root, actual);
}

TEST(operationsIntegrationTest, OpenFileRead_Success)
{
	// Arrange
	const auto first = GenerateUniqueTempPath();
	{
		auto writeStream = OpenFileWrite(first);
	}

	// Act
	auto stream = OpenFileRead(first);

	// Assert
	EXPECT_TRUE(stream);
}

TEST(operationsIntegrationTest, OpenFileRead_FailsIfNotExists)
{
	// Arrange
	const auto first = GenerateUniqueTempPath();

	// Act
	const auto stream = OpenFileRead(first);

	// Assert
	EXPECT_FALSE(stream);
}

TEST(operationsIntegrationTest, OpenFileWrite_Success)
{
	// Arrange
	const auto first = GenerateUniqueTempPath();

	// Act
	auto writeStream = OpenFileWrite(first);

	// Assert
	EXPECT_TRUE(writeStream);
}

TEST(operationsIntegrationTest, Exists_Success)
{
	// Arrange
	const auto first = GenerateUniqueTempPath();
	{
		auto writeStream = OpenFileWrite(first);
		ASSERT_TRUE(writeStream);
	}

	// Act
	// Assert
	EXPECT_TRUE(Exists(first));
}

TEST(operationsIntegrationTest, Exists_FalseIfNotExists)
{
	// Arrange
	const auto first = GenerateUniqueTempPath();
	// Act
	// Assert
	EXPECT_FALSE(Exists(first));
}

TEST(operationsIntegrationTest, Remove_Success)
{
	// Arrange
	const auto first = GenerateUniqueTempPath();
	{
		auto writeStream = OpenFileWrite(first);
		ASSERT_TRUE(writeStream);
	}

	// Act
	Remove(first);

	// Assert
	EXPECT_FALSE(Exists(first));
}

TEST(operationsIntegrationTest, RemoveAll_Success)
{
	// Arrange
	const auto first = GenerateUniqueTempPath();
	const auto subDirectory = first / "sub";
	CreateDirectories(subDirectory);
	const auto filePath = subDirectory / "file.dat";
	{
		auto writeStream = OpenFileWrite(filePath);
		ASSERT_TRUE(writeStream);
	}

	// Act
	RemoveAll(first);

	// Assert
	EXPECT_FALSE(Exists(first));
}

}
}
}
}
}
