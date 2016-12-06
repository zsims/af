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
	const auto id = repo.AddPath(path, boost::none);
	const auto id2 = repo.AddPath(path2, id);

	// Assert
	EXPECT_NE(id, id2);
}

TEST_F(FilePathRepositoryIntegrationTest, AddPath_FailsIfDuplicatePath)
{
	// Arrange
	FilePathRepository repo(*_connection);
	const fs::WindowsPath path(R"(C:\Foo\Bar)");

	// Act
	const auto id = repo.AddPath(path, boost::none);

	// Assert
	EXPECT_THROW(repo.AddPath(path, boost::none), AddFilePathFailedException);
}

TEST_F(FilePathRepositoryIntegrationTest, FindPath_Success)
{
	// Arrange
	FilePathRepository repo(*_connection);
	const fs::WindowsPath path(R"(C:\Foo\Bar)");
	const fs::WindowsPath path2(R"(D:\ding\bat)");
	const auto id = repo.AddPath(path, boost::none);

	// Act
	const auto found = repo.FindPath(path);
	const auto notFound = repo.FindPath(path2);

	// Assert
	ASSERT_TRUE(found);
	EXPECT_EQ(id, found.value());
	EXPECT_FALSE(notFound);
}


}
}
}
}
