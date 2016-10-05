#include "bslib/exceptions.hpp"
#include "bslib/BackupDatabase.hpp"
#include "bslib/blob/exceptions.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileAdder.hpp"
#include "bslib/file/fs/operations.hpp"
#include "TestBase.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace test {

class BackupDatabaseIntegrationTest : public TestBase
{
};

TEST_F(BackupDatabaseIntegrationTest, CreateSuccess)
{
	// Arrange
	// Act
	_testBackupDatabase.Create();
	// Assert
	EXPECT_TRUE(boost::filesystem::exists(_testBackupDatabase.GetBackupDatabaseDbPath()));
}

TEST_F(BackupDatabaseIntegrationTest, CreateThrowsIfExists)
{
	// Arrange
	CreateFile(_testBackupDatabase.GetBackupDatabaseDbPath());

	// Act
	// Assert
	EXPECT_THROW(_testBackupDatabase.Create(), DatabaseAlreadyExistsException);
}

TEST_F(BackupDatabaseIntegrationTest, OpenThrowsIfNotExists)
{
	// Arrange
	// Act
	// Assert
	EXPECT_THROW(_testBackupDatabase.Open(), DatabaseNotFoundException);
}

TEST_F(BackupDatabaseIntegrationTest, UnitOfWorkCommit)
{
	// Arrange
	_testBackupDatabase.Create();
	auto& database = _testBackupDatabase.GetBackupDatabase();
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

TEST_F(BackupDatabaseIntegrationTest, CreateUnitOfWorkTwiceFails)
{
	// Arrange
	_testBackupDatabase.Create();
	auto& forest = _testBackupDatabase.GetBackupDatabase();

	// Act
	// Assert
	// No support for multiple connections yet, so this isn't possible
	blob::NullBlobStore store;
	auto uow1 = forest.CreateUnitOfWork(store);
	EXPECT_THROW(forest.CreateUnitOfWork(store), std::runtime_error);
}

TEST_F(BackupDatabaseIntegrationTest, UnitOfWorkImplicitRollback)
{
	// Arrange
	_testBackupDatabase.Create();
	auto& forest = _testBackupDatabase.GetBackupDatabase();
	blob::NullBlobStore store;

	// Act
	const auto targetPath = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	{
		auto uow = forest.CreateUnitOfWork(store);
		auto adder = uow->CreateFileAdder();
		file::fs::CreateDirectories(targetPath);
		adder->Add(targetPath.ToString());
	}

	// Assert
	{
		auto uow = forest.CreateUnitOfWork(store);
		auto finder = uow->CreateFileFinder();
		EXPECT_FALSE(finder->FindLastChangedEventByPath(targetPath));
	}
}

TEST_F(BackupDatabaseIntegrationTest, OpenOrCreateExisting)
{
	// Arrange
	_testBackupDatabase.Create();
	_testBackupDatabase.Close();
	BackupDatabase secondBackupDatabase(_testBackupDatabase.GetBackupDatabaseDbPath());

	// Act
	// Assert
	ASSERT_NO_THROW(secondBackupDatabase.OpenOrCreate());
}

TEST_F(BackupDatabaseIntegrationTest, OpenOrCreateNew)
{
	// Arrange
	// Act
	// Assert
	ASSERT_NO_THROW(_testBackupDatabase.OpenOrCreate());
}

}
}
}
