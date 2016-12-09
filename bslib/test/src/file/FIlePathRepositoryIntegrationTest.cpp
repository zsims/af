#include "bslib/file/FilePathRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"
#include "bslib_test_util/TestBase.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace file {
namespace test {

class FilePathRepositoryIntegrationTest : public bslib_test_util::TestBase
{
protected:
	FilePathRepositoryIntegrationTest()
	{
		_testBackup.Create();
		_connection = _testBackup.ConnectToDatabase();
	}

	std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
};

TEST_F(FilePathRepositoryIntegrationTest, AddPath_Success)
{
	// Arrange
	FilePathRepository repo(*_connection);
	const fs::WindowsPath path(R"(C:\Foo\Bar)");
	const fs::WindowsPath path2(R"(c:\Foo\Bar\Baz)");

	// Act
	const auto id = repo.AddPath(path, FileType::Directory, boost::none);
	const auto id2 = repo.AddPath(path2, FileType::Directory, id);

	// Assert
	EXPECT_NE(id, id2);
}

TEST_F(FilePathRepositoryIntegrationTest, AddPath_FailsIfDuplicatePath)
{
	// Arrange
	FilePathRepository repo(*_connection);
	const fs::WindowsPath path(R"(C:\Foo\Bar)");

	// Act
	const auto id = repo.AddPath(path, FileType::Directory, boost::none);

	// Assert
	EXPECT_THROW(repo.AddPath(path, FileType::Directory, boost::none), AddFilePathFailedException);
}

TEST_F(FilePathRepositoryIntegrationTest, AddPath_SuccessIfDuplicatePathDifferentType)
{
	// Arrange
	FilePathRepository repo(*_connection);
	const fs::WindowsPath path(R"(C:\Foo\Bar)");

	// Act
	const auto id = repo.AddPath(path, FileType::Directory, boost::none);

	// Assert
	EXPECT_NO_THROW(repo.AddPath(path, FileType::RegularFile, boost::none));
}

TEST_F(FilePathRepositoryIntegrationTest, AddPathTree_Success)
{
	// Arrange
	FilePathRepository repo(*_connection);
	const fs::WindowsPath path(R"(C:\Foo\Bar)");

	// Act
	const auto id = repo.AddPathTree(path, FileType::RegularFile);

	// Assert
	const auto c = repo.FindPathDetails(fs::WindowsPath(R"(C:\)"), FileType::Directory);
	const auto cFoo = repo.FindPathDetails(fs::WindowsPath(R"(C:\Foo\)"), FileType::Directory);
	const auto cFooBar = repo.FindPathDetails(fs::WindowsPath(R"(C:\Foo\Bar)"), FileType::RegularFile);

	ASSERT_TRUE(c);
	EXPECT_FALSE(c->parentId);

	ASSERT_TRUE(cFoo);
	ASSERT_TRUE(cFoo->parentId);
	EXPECT_EQ(c->pathId, cFoo->parentId.value());

	ASSERT_TRUE(cFooBar);
	ASSERT_TRUE(cFooBar->parentId);
	EXPECT_EQ(cFoo->pathId, cFooBar->parentId.value());
	EXPECT_EQ(cFooBar->pathId, id);
}

TEST_F(FilePathRepositoryIntegrationTest, AddPathTree_DifferentTypesSuccess)
{
	// Arrange
	FilePathRepository repo(*_connection);
	const fs::WindowsPath path(R"(C:\Foo\Bar)");

	// Act
	FilePathRepository::cache_type cache;
	const auto directoryId = repo.AddPathTree(path, FileType::Directory, cache);
	const auto fileId = repo.AddPathTree(path, FileType::RegularFile, cache);

	// Assert
	const auto foundFile = repo.FindPathDetails(path, FileType::RegularFile);
	const auto foundDirectory = repo.FindPathDetails(path, FileType::Directory);

	EXPECT_NE(directoryId, fileId);
	ASSERT_TRUE(foundFile);
	EXPECT_EQ(fileId, foundFile->pathId);
	EXPECT_EQ(FileType::RegularFile, foundFile->type);

	ASSERT_TRUE(foundDirectory);
	EXPECT_EQ(directoryId, foundDirectory->pathId);
	EXPECT_EQ(FileType::Directory, foundDirectory->type);

	const auto foundFoo = repo.FindPathDetails(fs::NativePath(R"(C:\Foo\)"), FileType::Directory);
	ASSERT_TRUE(foundFoo);
	EXPECT_EQ(foundFoo->pathId, foundFile->parentId);
	EXPECT_EQ(foundFoo->pathId, foundDirectory->parentId);
}

TEST_F(FilePathRepositoryIntegrationTest, FindPath_Success)
{
	// Arrange
	FilePathRepository repo(*_connection);
	const fs::WindowsPath path(R"(C:\Foo\Bar)");
	const fs::WindowsPath path2(R"(D:\ding\bat)");
	const auto id = repo.AddPath(path, FileType::Directory, boost::none);

	// Act
	const auto found = repo.FindPath(path, FileType::Directory);
	const auto notFound = repo.FindPath(path2, FileType::Directory);

	// Assert
	ASSERT_TRUE(found);
	EXPECT_EQ(id, found.value());
	EXPECT_FALSE(notFound);
}

}
}
}
}
