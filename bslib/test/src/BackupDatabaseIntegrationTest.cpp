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
	auto store = std::make_shared<blob::NullBlobStore>();

	const auto targetPath = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	{
		auto uow = database.CreateUnitOfWork(store);
		auto adder = uow->CreateFileAdder(Uuid::Empty);
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
	auto store = std::make_shared<blob::NullBlobStore>();
	auto uow1 = backup.CreateUnitOfWork(store);
	EXPECT_NO_THROW(backup.CreateUnitOfWork(store));
}

TEST_F(BackupDatabaseIntegrationTest, UnitOfWorkImplicitRollback)
{
	// Arrange
	_testBackup.Create();
	auto& backup = _testBackup.GetBackupDatabase();
	auto store = std::make_shared<blob::NullBlobStore>();

	// Act
	const auto targetPath = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	{
		auto uow = backup.CreateUnitOfWork(store);
		auto adder = uow->CreateFileAdder(Uuid::Empty);
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

TEST_F(BackupDatabaseIntegrationTest, SaveAs_Success)
{
	// Arrange
	const auto target = GetUniqueTempPath();
	_testBackup.OpenOrCreate();

	{
		const auto uow = _testBackup.GetBackup().CreateUnitOfWork();
		auto adder = uow->CreateFileAdder(Uuid::Empty);
		const auto testFile = GetUniqueExtendedTempPath();
		WriteFile(testFile, "hi");
		adder->Add(testFile.ToString());
		uow->Commit();
	}

	// Act
	_testBackup.GetBackupDatabase().SaveAs(target);

	// Assert
	ASSERT_TRUE(boost::filesystem::exists(target));
	BackupDatabase copy(target);
	ASSERT_NO_THROW(copy.Open());
	auto nullBlobStore = std::make_shared<blob::NullBlobStore>();
	const auto uowCopy = copy.CreateUnitOfWork(nullBlobStore);
	const auto finder = uowCopy->CreateFileFinder();
	const auto allEvents = finder->GetAllEvents();
	EXPECT_EQ(1, allEvents.size());
}

TEST_F(BackupDatabaseIntegrationTest, SaveAs_ExcludesUncommittedWork)
{
	// Arrange
	const auto target = GetUniqueTempPath();
	_testBackup.OpenOrCreate();

	const auto uow = _testBackup.GetBackup().CreateUnitOfWork();
	auto adder = uow->CreateFileAdder(Uuid::Empty);
	const auto testFile = GetUniqueExtendedTempPath();
	WriteFile(testFile, "hi");
	adder->Add(testFile.ToString());

	// Act
	_testBackup.GetBackupDatabase().SaveAs(target);

	// Assert
	ASSERT_TRUE(boost::filesystem::exists(target));
	BackupDatabase copy(target);
	copy.Open();
	auto nullBlobStore = std::make_shared<blob::NullBlobStore>();
	const auto uowCopy = copy.CreateUnitOfWork(nullBlobStore);
	const auto finder = uowCopy->CreateFileFinder();
	const auto allEvents = finder->GetAllEvents();
	EXPECT_EQ(0, allEvents.size());
}

TEST_F(BackupDatabaseIntegrationTest, SaveAs_ThrowsIfExists)
{
	// Arrange
	const auto target = GetUniqueTempPath();
	_testBackup.OpenOrCreate();
	WriteFile(target);

	// Act
	// Assert
	ASSERT_THROW(_testBackup.GetBackupDatabase().SaveAs(target), DatabaseAlreadyExistsException);
}

}
}
}
