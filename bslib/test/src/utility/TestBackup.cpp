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
	, _forestPath(baseDir / "forest.db")
{
}

void TestBackup::Open()
{
	_forest = std::make_unique<bslib::BackupDatabase>(_forestPath, std::make_unique<blob::DirectoryBlobStore>(_baseDir));
	_forest->Open();
}

void TestBackup::Create()
{
	_forest = std::make_unique<bslib::BackupDatabase>(_forestPath, std::make_unique<blob::DirectoryBlobStore>(_baseDir));
	_forest->Create();
}

void TestBackup::OpenOrCreate()
{
	_forest = std::make_unique<bslib::BackupDatabase>(_forestPath, std::make_unique<blob::DirectoryBlobStore>(_baseDir));
	_forest->OpenOrCreate();
}

void TestBackup::OpenWithNullStore()
{
	_forest = std::make_unique<bslib::BackupDatabase>(_forestPath, std::make_unique<blob::NullBlobStore>());
	_forest->Open();
}

void TestBackup::CreateWithNullStore()
{
	_forest = std::make_unique<bslib::BackupDatabase>(_forestPath, std::make_unique<blob::NullBlobStore>());
	_forest->Create();
}
	
std::unique_ptr<sqlitepp::ScopedSqlite3Object> TestBackup::ConnectToDatabase() const
{
	auto connection = std::make_unique<sqlitepp::ScopedSqlite3Object>();
	sqlitepp::open_database_or_throw(WideToUTF8String(_forestPath.wstring()).c_str(), *connection, SQLITE_OPEN_READWRITE);
	sqlitepp::exec_or_throw(*connection, "PRAGMA case_sensitive_like = true;");
	return connection;
}

af::bslib::BackupDatabase& TestBackup::GetBackupDatabase()
{
	return *_forest;
}

void TestBackup::Close()
{
	_forest.reset();
}

}
}
}
}
