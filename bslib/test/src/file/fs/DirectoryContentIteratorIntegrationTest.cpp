#include "bslib/file/fs/operations.hpp"
#include "bslib/file/fs/DirectoryContentIterator.hpp"
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

namespace {
void Touch(const WindowsPath& path)
{
	const auto widePath = path.ToExtendedWideString();
	std::ofstream f(widePath.c_str(), std::ofstream::out | std::ofstream::binary);
}
}

TEST(DirectoryContentIteratorIntegrationTest, Iterate_Success)
{
	// Arrange
	const auto temp = GenerateUniqueTempPath();
	ASSERT_TRUE(CreateDirectorySexy(temp));
	const auto file1 = temp / "file.dat";
	Touch(file1);
	const auto file2 = temp / "other.dat";
	Touch(file2);
	// Dictories will have trailing slashes
	const auto directory = (temp / "a directory").EnsureTrailingSlash();
	ASSERT_TRUE(CreateDirectorySexy(directory));

	// Act
	boost::system::error_code ec;
	DirectoryContentIterator it(temp, ec);
	ASSERT_FALSE(ec);
	std::vector<WindowsPath> foundPaths;
	for (const auto& p : it)
	{
		foundPaths.push_back(p.fullPath);
	}

	// Assert
	EXPECT_THAT(foundPaths, testing::UnorderedElementsAre(
		file1,
		file2,
		directory,
		temp / ".",
		temp / ".."
	));
}

TEST(DirectoryContentIteratorIntegrationTest, Iterate_ExtendedPathSuccess)
{
	// Arrange
	const auto temp = GenerateUniqueTempPath() / UTF8String(150, 'a') / UTF8String(150, 'b');
	ASSERT_TRUE(CreateDirectories(temp));
	const auto file1 = temp / "file.dat";
	Touch(file1);
	const auto file2 = temp / "other.dat";
	Touch(file2);
	// Dictories will have trailing slashes
	const auto directory = (temp / "a directory").EnsureTrailingSlash();
	ASSERT_TRUE(CreateDirectorySexy(directory));

	// Act
	boost::system::error_code ec;
	DirectoryContentIterator it(temp, ec);
	ASSERT_FALSE(ec);
	std::vector<WindowsPath> foundPaths;
	for (const auto& p : it)
	{
		foundPaths.push_back(p.fullPath);
	}

	// Assert
	EXPECT_THAT(foundPaths, testing::UnorderedElementsAre(
		file1,
		file2,
		directory,
		temp / ".",
		temp / ".."
	));
}

TEST(DirectoryContentIteratorIntegrationTest, Iterate_RootSuccess)
{
	// Arrange
	// Iterating the root directory has different rules (e.g. \\?\C:\) per https://msdn.microsoft.com/en-us/library/windows/desktop/aa364419(v=vs.85).aspx
	const auto temp = GenerateUniqueTempPath();
	const auto root = temp.GetIntermediatePaths()[0];

	// Act
	boost::system::error_code ec;
	DirectoryContentIterator it(root, ec);
	ASSERT_FALSE(ec);
	std::vector<WindowsPath> foundPaths;
	for (const auto& p : it)
	{
		foundPaths.push_back(p.fullPath);
	}

	// Assert
	EXPECT_FALSE(foundPaths.empty());
}

TEST(DirectoryContentIteratorIntegrationTest, Iterate_NotExistFails)
{
	// Arrange
	// Iterating the root directory has different rules (e.g. \\?\C:\) per https://msdn.microsoft.com/en-us/library/windows/desktop/aa364419(v=vs.85).aspx
	const auto temp = GenerateUniqueTempPath();

	// Act
	boost::system::error_code ec;
	DirectoryContentIterator it(temp, ec);

	// Assert
	EXPECT_TRUE(ec);
}

}
}
}
}
}
