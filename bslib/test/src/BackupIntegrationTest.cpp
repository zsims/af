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
	blob::BlobStoreManager blobStoreManager;
	Backup secondBackup(_testBackup.GetBackupDatabaseDbPath(), "TEST", blobStoreManager);

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

TEST_F(BackupIntegrationTest, SaveDatabaseCopy_SavesToBlobStore)
{
	// Arrange
	_testBackup.OpenOrCreate();
	const auto expectedPath = _testBackup.GetDirectoryStorePath() / (_testBackup.GetName() + ".db");

	// Act
	_testBackup.GetBackup().SaveDatabaseCopy();

	// Assert
}

TEST_F(BackupIntegrationTest, ModifyBlobStoresDuringBackup_Success)
{
	// Arrange
	_testBackup.OpenOrCreate();
	auto& blobStoreManager = _testBackup.GetBlobStoreManager();
	auto uow = _testBackup.GetBackup().CreateUnitOfWork();
	auto adder = uow->CreateFileAdder();
	const auto tempPath = GetUniqueExtendedTempPath();
	const auto blobAddress = WriteFile(tempPath, "hey");
	adder->Add(tempPath.ToString());

	// Act
	blobStoreManager.RemoveById((*(blobStoreManager.GetStores().begin()))->GetId());

	// Assert
	ASSERT_NO_THROW(uow->GetBlob(blobAddress));
}

}
}
}
