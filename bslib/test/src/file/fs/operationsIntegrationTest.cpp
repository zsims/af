#include "bslib/file/fs/operations.hpp"
#include "bslib/file/exceptions.hpp"
#include "file/test_utility/ScopedWorkingDirectory.hpp"
#include "test_util/TestBase.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fstream>
#include <string>

namespace af {
namespace bslib {
namespace file {
namespace fs {
namespace test {

class operationsIntegrationTest : public test_util::TestBase
{
};

TEST_F(operationsIntegrationTest, IsDirectory_Success)
{
	// Arrange
	const auto first = GetUniqueExtendedTempPath();
	const auto parent = first.ParentPathCopy();

	// Act
	const auto isDirectory = IsDirectory(parent);

	// Assert
	EXPECT_TRUE(isDirectory);
}

TEST_F(operationsIntegrationTest, IsDirectory_NotExistSuccess)
{
	// Arrange
	const auto first = GetUniqueExtendedTempPath();

	// Act
	boost::system::error_code ec;
	const auto isDirectory = IsDirectory(first, ec);

	// Assert
	EXPECT_FALSE(isDirectory);
	EXPECT_TRUE(ec);
}

TEST_F(operationsIntegrationTest, IsRegularFile_Success)
{
	// Arrange
	const auto first = GetUniqueExtendedTempPath();
	std::ofstream f(UTF8ToWideString(first.ToExtendedString()), std::ofstream::out | std::ofstream::binary);

	// Act
	const auto isFile = IsRegularFile(first);

	// Assert
	EXPECT_TRUE(isFile);
}

TEST_F(operationsIntegrationTest, IsRegularFile_NotExistSuccess)
{
	// Arrange
	const auto first = GetUniqueExtendedTempPath();

	// Act
	boost::system::error_code ec;
	const auto isDirectory = IsRegularFile(first, ec);

	// Assert
	EXPECT_FALSE(isDirectory);
	EXPECT_TRUE(ec);
}

TEST_F(operationsIntegrationTest, IsRegularFile_WithDirectoryFalse)
{
	// Arrange
	const auto first = GetUniqueExtendedTempPath();
	const auto parent = first.ParentPathCopy();

	// Act
	boost::system::error_code ec;
	const auto isFile = IsRegularFile(parent, ec);

	// Assert
	EXPECT_FALSE(isFile);
	EXPECT_FALSE(ec);
}

TEST_F(operationsIntegrationTest, CreateDirectorySexy_Success)
{
	// Arrange
	const auto unique = GetUniqueExtendedTempPath();

	// Act
	ASSERT_TRUE(CreateDirectorySexy(unique));

	// Assert
	const auto isDirectory = IsDirectory(unique);
	EXPECT_TRUE(isDirectory);
}

TEST_F(operationsIntegrationTest, CreateDirectorySexy_AlreadyExistingSuccess)
{
	// Arrange
	const auto unique = GetUniqueExtendedTempPath();
	CreateDirectorySexy(unique);

	// Act
	// Assert
	boost::system::error_code ec;
	EXPECT_FALSE(CreateDirectorySexy(unique, ec));
	EXPECT_TRUE(ec);
}

TEST_F(operationsIntegrationTest, CreateDirectories_Success)
{
	// Arrange
	const auto unique = GetUniqueExtendedTempPath();
	const auto full = unique / "and" / u8"我这样做对吗" / "deeper";

	// Act
	EXPECT_TRUE(CreateDirectories(full));

	// Assert
	EXPECT_TRUE(IsDirectory(full));
}

TEST_F(operationsIntegrationTest, CreateDirectories_ExtendedPathSuccess)
{
	// Arrange
	const auto unique = GetUniqueExtendedTempPath();
	const auto full = unique / "and" / UTF8String(150, 'a') / "deeper" / UTF8String(150, 'b') / u8"我";

	// Act
	ASSERT_TRUE(CreateDirectories(full));

	// Assert
	EXPECT_TRUE(IsDirectory(full));
}

TEST_F(operationsIntegrationTest, CreateDirectories_AlreadyExistsSuccess)
{
	// Arrange
	const auto unique = GetUniqueExtendedTempPath();
	ASSERT_TRUE(CreateDirectorySexy(unique));

	// Act
	// Assert
	boost::system::error_code ec;
	EXPECT_FALSE(CreateDirectories(unique, ec));
	EXPECT_FALSE(ec);
}

TEST_F(operationsIntegrationTest, CreateDirectories_FileExistsFails)
{
	// Arrange
	boost::system::error_code ec;
	const auto unique = GetUniqueExtendedTempPath();
	const auto widePath = UTF8ToWideString(unique.ToExtendedString());
	std::ofstream f(widePath.c_str(), std::ofstream::out | std::ofstream::binary);

	// Act
	// Assert
	EXPECT_FALSE(CreateDirectories(unique, ec));
	EXPECT_TRUE(ec);
}

TEST_F(operationsIntegrationTest, GetAbsolutePath_Success)
{
	// Arrange
	const auto expected = NativeFromBoostPath(GetUniqueTempPath());
	const auto name = expected.GetFilename();
	const auto relativePath = name + R"(\..\)" + name;
	test_utility::ScopedWorkingDirectory workingDirectory(expected / "..");

	// Act
	const auto actual = GetAbsolutePath(relativePath);

	// Assert
	EXPECT_EQ(expected, actual);
}

TEST_F(operationsIntegrationTest, GetAbsolutePath_RootSuccess)
{
	// Arrange
	const auto unique = NativeFromBoostPath(GetUniqueTempPath());
	const auto root = unique.GetIntermediatePaths()[0];
	test_utility::ScopedWorkingDirectory workingDirectory(root);

	// Act
	auto actual = GetAbsolutePath(".");
	actual.EnsureTrailingSlash();

	// Assert
	EXPECT_EQ(root, actual);
}

TEST_F(operationsIntegrationTest, OpenFileRead_Success)
{
	// Arrange
	const auto first = GetUniqueExtendedTempPath();
	{
		auto writeStream = OpenFileWrite(first);
	}

	// Act
	auto stream = OpenFileRead(first);

	// Assert
	EXPECT_TRUE(stream);
}

TEST_F(operationsIntegrationTest, OpenFileRead_FailsIfNotExists)
{
	// Arrange
	const auto first = GetUniqueExtendedTempPath();

	// Act
	const auto stream = OpenFileRead(first);

	// Assert
	EXPECT_FALSE(stream);
}

TEST_F(operationsIntegrationTest, OpenFileWrite_Success)
{
	// Arrange
	const auto first = GetUniqueExtendedTempPath();

	// Act
	auto writeStream = OpenFileWrite(first);

	// Assert
	EXPECT_TRUE(writeStream);
}

TEST_F(operationsIntegrationTest, Exists_Success)
{
	// Arrange
	const auto first = GetUniqueExtendedTempPath();
	{
		auto writeStream = OpenFileWrite(first);
		ASSERT_TRUE(writeStream);
	}

	// Act
	// Assert
	EXPECT_TRUE(Exists(first));
}

TEST_F(operationsIntegrationTest, Exists_FalseIfNotExists)
{
	// Arrange
	const auto first = GetUniqueExtendedTempPath();
	// Act
	// Assert
	EXPECT_FALSE(Exists(first));
}

TEST_F(operationsIntegrationTest, Remove_Success)
{
	// Arrange
	const auto first = GetUniqueExtendedTempPath();
	{
		auto writeStream = OpenFileWrite(first);
		ASSERT_TRUE(writeStream);
	}

	// Act
	Remove(first);

	// Assert
	EXPECT_FALSE(Exists(first));
}

TEST_F(operationsIntegrationTest, RemoveAll_Success)
{
	// Arrange
	const auto first = GetUniqueExtendedTempPath();
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
