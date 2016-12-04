#include "bslib/file/fs/WindowsPath.hpp"

#include "bslib/file/exceptions.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string>

namespace af {
namespace bslib {
namespace file {
namespace fs {
namespace test {

TEST(WindowsPathIntegrationTest, EnsureTrailingSlash_Success)
{
	// Arrange
	WindowsPath input(R"(C:\this needs\ a slash)");

	// Act
	input.EnsureTrailingSlash();

	// Assert
	const UTF8String expected(R"(\\?\C:\this needs\ a slash\)");
	EXPECT_EQ(input, expected);
}

TEST(WindowsPathIntegrationTest, EnsureTrailingSlashCopy_Success)
{
	// Arrange
	const WindowsPath input(R"(C:\this needs\ a slash)");

	// Act
	const auto actual = input.EnsureTrailingSlashCopy();

	// Assert
	const UTF8String expected(R"(\\?\C:\this needs\ a slash\)");
	EXPECT_EQ(actual, expected);
}

TEST(WindowsPathIntegrationTest, ParentPathCopy_Success)
{
	// Arrange
	const WindowsPath input(R"(C:\i am the father\woooooo)");

	// Act
	const auto actual = input.ParentPathCopy();

	// Assert
	const UTF8String expected(R"(\\?\C:\i am the father)");
	EXPECT_EQ(actual, expected);
}

TEST(WindowsPathIntegrationTest, GetFilename_Success)
{
	// Arrange
	const WindowsPath input(u8R"(C:\something\文件名.dat)");

	// Act
	const auto actual = input.GetFilename();

	// Assert
	const UTF8String expected(u8"文件名.dat");
	EXPECT_EQ(actual, expected);
}

TEST(WindowsPathIntegrationTest, ToExtendedString_Success)
{
	// Arrange
	const WindowsPath input(R"(C:\something\file.dat)");

	// Act
	const auto actual = input.ToExtendedString();

	// Assert
	const UTF8String expected(R"(\\?\C:\something\file.dat)");
	EXPECT_EQ(actual, expected);
}

TEST(WindowsPathIntegrationTest, ToNormalString_Success)
{
	// Arrange
	const WindowsPath input(R"(C:\something\file.dat)");

	// Act
	const auto actual = input.ToNormalString();

	// Assert
	const UTF8String expected(R"(C:\something\file.dat)");
	EXPECT_EQ(actual, expected);
}

TEST(WindowsPathIntegrationTest, GetIntermediatePaths_FilePathSuccess)
{
	// Arrange
	const WindowsPath input(R"(C:\something\with\file.dat)");

	// Act
	const auto result = input.GetIntermediatePaths();

	// Assert
	EXPECT_THAT(result, ::testing::ElementsAre(
		WindowsPath(R"(\\?\C:\)"),
		WindowsPath(R"(\\?\C:\something\)"),
		WindowsPath(R"(\\?\C:\something\with\)"),
		WindowsPath(R"(\\?\C:\something\with\file.dat)")
	));
}

TEST(WindowsPathIntegrationTest, GetIntermediatePaths_HandlesEmptySegmentsSuccess)
{
	// Arrange
	const WindowsPath input(R"(C:\something\\empty\\\parts)");

	// Act
	const auto result = input.GetIntermediatePaths();

	// Assert
	EXPECT_THAT(result, ::testing::ElementsAre(
		WindowsPath(R"(\\?\C:\)"),
		WindowsPath(R"(\\?\C:\something\)"),
		WindowsPath(R"(\\?\C:\something\\)"),
		WindowsPath(R"(\\?\C:\something\\empty\)"),
		WindowsPath(R"(\\?\C:\something\\empty\\)"),
		WindowsPath(R"(\\?\C:\something\\empty\\\)"),
		WindowsPath(R"(\\?\C:\something\\empty\\\parts)")
	));
}

TEST(WindowsPathIntegrationTest, GetIntermediatePaths_DirectoryPathSuccess)
{
	// Arrange
	const WindowsPath input(R"(C:\something\with\dir\)");

	// Act
	const auto result = input.GetIntermediatePaths();

	// Assert
	EXPECT_THAT(result, ::testing::ElementsAre(
		WindowsPath(R"(\\?\C:\)"),
		WindowsPath(R"(\\?\C:\something\)"),
		WindowsPath(R"(\\?\C:\something\with\)"),
		WindowsPath(R"(\\?\C:\something\with\dir\)")
	));
}

TEST(WindowsPathIntegrationTest, GetIntermediatePaths_IgnoresForwardSlashes)
{
	// Arrange
	const WindowsPath input(R"(C:\something\wow/sir\)");

	// Act
	const auto result = input.GetIntermediatePaths();

	// Assert
	EXPECT_THAT(result, ::testing::ElementsAre(
		WindowsPath(R"(\\?\C:\)"),
		WindowsPath(R"(\\?\C:\something\)"),
		WindowsPath(R"(\\?\C:\something\wow/sir\)")
	));
}

TEST(WindowsPathIntegrationTest, GetIntermediatePaths_RootPathSuccess)
{
	// Arrange
	const WindowsPath input(R"(C:)");

	// Act
	const auto result = input.GetIntermediatePaths();

	// Assert
	EXPECT_THAT(result, ::testing::ElementsAre(
		WindowsPath(R"(\\?\C:)")
	));
}

TEST(WindowsPathIntegrationTest, GetIntermediatePaths_EmptySuccess)
{
	// Arrange
	const WindowsPath input("");

	// Act
	const auto result = input.GetIntermediatePaths();

	// Assert
	EXPECT_THAT(result, ::testing::ElementsAre(
		WindowsPath(R"(\\?\)")
	));
}

TEST(WindowsPathIntegrationTest, AppendSegment_Success)
{
	// Arrange
	WindowsPath input(R"(C:\wow this is)");

	// Act
	input.AppendSegment(u8"great");

	// Assert
	const UTF8String expected(R"(\\?\C:\wow this is\great)");
	EXPECT_EQ(input, expected);
}

TEST(WindowsPathIntegrationTest, AppendSegment_WithSlashSuccess)
{
	// Arrange
	WindowsPath input(R"(C:\wow this is\)");

	// Act
	input.AppendSegment(u8"great");
	input.AppendSegment(u8"HA");

	// Assert
	const UTF8String expected(R"(\\?\C:\wow this is\great\HA)");
	EXPECT_EQ(input, expected);
}

TEST(WindowsPathIntegrationTest, AppendFull_Success)
{
	// Arrange
	WindowsPath input(R"(C:\foo\bar)");

	// Act
	input.AppendFull(WindowsPath(R"(D:\fizz\buzz)"));

	// Assert
	const UTF8String expected(R"(\\?\C:\foo\bar\D\fizz\buzz)");
	EXPECT_EQ(input, expected);
}

TEST(WindowsPathIntegrationTest, AppendFull_EmptySuccess)
{
	// Arrange
	WindowsPath input(R"(C:\foo\bar)");

	// Act
	input.AppendFull(WindowsPath());

	// Assert
	const UTF8String expected(R"(\\?\C:\foo\bar)");
	EXPECT_EQ(input, expected);
}

TEST(WindowsPathIntegrationTest, GetDepth_Success)
{
	// Arrange
	const WindowsPath blank("");
	const WindowsPath root(u8R"(C:\)");
	const WindowsPath rootNoTrail(u8R"(C:)");
	const WindowsPath path(u8R"(C:\文件名 here\)");
	const WindowsPath pathNoTrail(u8R"(C:\文件名 here)");
	const WindowsPath deepPath(u8R"(C:\文件名 here\is deeper\)");
	const WindowsPath deepPathNoTrail(u8R"(C:\文件名 here\is deeper)");

	// Act
	// Assert
	EXPECT_EQ(0, blank.GetDepth());
	EXPECT_EQ(0, root.GetDepth());
	EXPECT_EQ(0, rootNoTrail.GetDepth());
	EXPECT_EQ(1, path.GetDepth());
	EXPECT_EQ(1, pathNoTrail.GetDepth());
	EXPECT_EQ(2, deepPath.GetDepth());
	EXPECT_EQ(2, deepPathNoTrail.GetDepth());
}

}
}
}
}
}
