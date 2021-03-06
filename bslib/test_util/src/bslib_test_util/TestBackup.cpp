#include "bslib_test_util/TestBackup.hpp"

#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/blob/DirectoryBlobStore.hpp"
#include "bslib/unicode.hpp"

namespace af {
namespace bslib_test_util {

TestBackup::TestBackup(const boost::filesystem::path& baseDir)
	: _baseDir(baseDir)
	, _backupDatabasePath(baseDir / "backup.db")
	, _name("test")
	, _blobStoreManager(baseDir / "stores.xml")
{
	_blobStoreManager.AddBlobStore(std::make_shared<bslib::blob::DirectoryBlobStore>(_baseDir));
}

void TestBackup::Open()
{
	_backup = std::make_unique<bslib::Backup>(_backupDatabasePath, _name, _blobStoreManager);
	_backup->Open();
}

void TestBackup::Create()
{
	_backup = std::make_unique<bslib::Backup>(_backupDatabasePath, _name, _blobStoreManager);
	_backup->Create();
}

bslib::Backup& TestBackup::OpenOrCreate()
{
	_backup = std::make_unique<bslib::Backup>(_backupDatabasePath, _name, _blobStoreManager);
	_backup->OpenOrCreate();
	return *_backup;
}

std::unique_ptr<bslib::sqlitepp::ScopedSqlite3Object> TestBackup::ConnectToDatabase() const
{
	auto connection = std::make_unique<bslib::sqlitepp::ScopedSqlite3Object>();
	bslib::sqlitepp::open_database_or_throw(bslib::WideToUTF8String(_backupDatabasePath.wstring()).c_str(), *connection, SQLITE_OPEN_READWRITE);
	bslib::sqlitepp::exec_or_throw(*connection, "PRAGMA case_sensitive_like = true;");
	return connection;
}

af::bslib::BackupDatabase& TestBackup::GetBackupDatabase()
{
	return _backup->GetBackupDatabase();
}

af::bslib::Backup& TestBackup::GetBackup()
{
	return *_backup;
}

bslib::blob::BlobStoreManager& TestBackup::GetBlobStoreManager()
{
	return _blobStoreManager;
}

void TestBackup::Close()
{
	_backup.reset();
}

}
}
