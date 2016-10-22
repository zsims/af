#include "blob/MockBlobStore.hpp"
#include "bslib/exceptions.hpp"
#include "bslib/Backup.hpp"
#include "bslib/blob/exceptions.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/fs/operations.hpp"
#include "bslib_test_util/TestBase.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace test {

class BackupIntegrationTest : public bslib_test_util::TestBase
{
};

TEST_F(BackupIntegrationTest, Open_ThrowsWithNoDb)
{
	// Arrange
	// Act
	EXPECT_THROW(_testBackup.Open(), DatabaseNotFoundException);
}

TEST_F(BackupIntegrationTest, OpenOrCreateExisting)
{
	// Arrange
	_testBackup.Create();
	_testBackup.Close();
	Backup secondBackup(_testBackup.GetBackupDatabaseDbPath(), "TEST");

	// Act
	// Assert
	ASSERT_NO_THROW(secondBackup.OpenOrCreate());
}

TEST_F(BackupIntegrationTest, OpenOrCreateNew)
{
	// Arrange
	// Act
	// Assert
	ASSERT_NO_THROW(_testBackup.OpenOrCreate());
}

TEST_F(BackupIntegrationTest, AddBlobStore_Success)
{
	// Arrange
	auto mockBlobStore = std::make_unique<blob::test::MockBlobStore>();

	// Setup expectations before transfering ownership, per https://groups.google.com/forum/#!topic/googlemock/lQ9y9SWzr0A
	EXPECT_CALL(*mockBlobStore, CreateBlob(::testing::_, ::testing::_)).Times(::testing::AtLeast(1));

	Backup backup(_testBackup.GetBackupDatabaseDbPath(), "TEST");
	backup.AddBlobStore(std::move(mockBlobStore));
	backup.OpenOrCreate();

	// Act
	const auto targetPath = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	{
		auto uow = backup.CreateUnitOfWork();
		auto adder = uow->CreateFileAdder();
		file::fs::CreateDirectories(targetPath);
		WriteFile(targetPath / "file.dat", "bruce");
		adder->Add(targetPath.ToString());
	}

	// Assert
	// See above EXPECT_CALL
}

TEST_F(BackupIntegrationTest, GetDefaultBackupDatabasePath_Success)
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
