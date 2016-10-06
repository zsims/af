#include "utility/TestBackup.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/blob/DirectoryBlobStore.hpp"
#include "bslib/unicode.hpp"

namespace af {
namespace bslib {
namespace test {
namespace utility {

TestBackup::TestBackup(const boost::filesystem::path& baseDir)
	: _baseDir(baseDir)
	, _backupDatabasePath(baseDir / "backup.db")
{
}

void TestBackup::Open()
{
	_backup = std::make_unique<bslib::Backup>(_backupDatabasePath, "Testing");
	_backup->AddBlobStore(std::make_unique<blob::DirectoryBlobStore>(_baseDir));
	_backup->Open();
}

void TestBackup::Create()
{
	_backup = std::make_unique<bslib::Backup>(_backupDatabasePath, "Testing");
	_backup->AddBlobStore(std::make_unique<blob::DirectoryBlobStore>(_baseDir));
	_backup->Create();
}

void TestBackup::OpenOrCreate()
{
	_backup = std::make_unique<bslib::Backup>(_backupDatabasePath, "Testing");
	_backup->AddBlobStore(std::make_unique<blob::DirectoryBlobStore>(_baseDir));
	_backup->OpenOrCreate();
}

std::unique_ptr<sqlitepp::ScopedSqlite3Object> TestBackup::ConnectToDatabase() const
{
	auto connection = std::make_unique<sqlitepp::ScopedSqlite3Object>();
	sqlitepp::open_database_or_throw(WideToUTF8String(_backupDatabasePath.wstring()).c_str(), *connection, SQLITE_OPEN_READWRITE);
	sqlitepp::exec_or_throw(*connection, "PRAGMA case_sensitive_like = true;");
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

void TestBackup::Close()
{
	_backup.reset();
}

}
}
}
}
