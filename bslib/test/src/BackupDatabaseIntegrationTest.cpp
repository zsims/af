#include "bslib/exceptions.hpp"
#include "bslib/BackupDatabase.hpp"
#include "bslib/blob/exceptions.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileAdder.hpp"
#include "bslib/file/fs/operations.hpp"
#include "bslib_test_util/TestBase.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace test {

class BackupDatabaseIntegrationTest : public bslib_test_util::TestBase
{
};

TEST_F(BackupDatabaseIntegrationTest, CreateSuccess)
{
	// Arrange
	// Act
	_testBackup.Create();
	// Assert
	EXPECT_TRUE(boost::filesystem::exists(_testBackup.GetBackupDatabaseDbPath()));
}

TEST_F(BackupDatabaseIntegrationTest, CreateThrowsIfExists)
{
	// Arrange
	WriteFile(_testBackup.GetBackupDatabaseDbPath());

	// Act
	// Assert
	EXPECT_THROW(_testBackup.Create(), DatabaseAlreadyExistsException);
}

TEST_F(BackupDatabaseIntegrationTest, OpenThrowsIfNotExists)
{
	// Arrange
	// Act
	// Assert
	EXPECT_THROW(_testBackup.Open(), DatabaseNotFoundException);
}

TEST_F(BackupDatabaseIntegrationTest, UnitOfWorkCommit)
{
	// Arrange
	_testBackup.Create();
	auto& database = _testBackup.GetBackupDatabase();
	blob::NullBlobStore store;

	const auto targetPath = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	{
		auto uow = database.CreateUnitOfWork(store);
		auto adder = uow->CreateFileAdder();
		file::fs::CreateDirectories(targetPath);
		adder->Add(targetPath.ToString());
		// Act
		uow->Commit();
	}

	// Assert
	{
		auto uow = database.CreateUnitOfWork(store);
		auto finder = uow->CreateFileFinder();
		EXPECT_TRUE(finder->FindLastChangedEventByPath(targetPath));
	}
}

TEST_F(BackupDatabaseIntegrationTest, CreateUnitOfWork_TwiceSuccess)
{
	// Arrange
	_testBackup.Create();
	auto& backup = _testBackup.GetBackupDatabase();

	// Act
	// Assert
	blob::NullBlobStore store;
	auto uow1 = backup.CreateUnitOfWork(store);
	EXPECT_NO_THROW(backup.CreateUnitOfWork(store));
}

TEST_F(BackupDatabaseIntegrationTest, UnitOfWorkImplicitRollback)
{
	// Arrange
	_testBackup.Create();
	auto& backup = _testBackup.GetBackupDatabase();
	blob::NullBlobStore store;

	// Act
	const auto targetPath = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	{
		auto uow = backup.CreateUnitOfWork(store);
		auto adder = uow->CreateFileAdder();
		file::fs::CreateDirectories(targetPath);
		adder->Add(targetPath.ToString());
	}

	// Assert
	{
		auto uow = backup.CreateUnitOfWork(store);
		auto finder = uow->CreateFileFinder();
		EXPECT_FALSE(finder->FindLastChangedEventByPath(targetPath));
	}
}

TEST_F(BackupDatabaseIntegrationTest, OpenOrCreateExisting)
{
	// Arrange
	_testBackup.Create();
	_testBackup.Close();
	BackupDatabase secondBackupDatabase(_testBackup.GetBackupDatabaseDbPath());

	// Act
	// Assert
	ASSERT_NO_THROW(secondBackupDatabase.OpenOrCreate());
}

TEST_F(BackupDatabaseIntegrationTest, OpenOrCreateNew)
{
	// Arrange
	// Act
	// Assert
	ASSERT_NO_THROW(_testBackup.OpenOrCreate());
}

}
}
}
