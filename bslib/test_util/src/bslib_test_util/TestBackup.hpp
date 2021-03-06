#pragma once

#include "bslib/Backup.hpp"
#include "bslib/BackupDatabase.hpp"
#include "bslib/blob/BlobStoreManager.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

#include <memory>

namespace af {
namespace bslib_test_util {

/**
 * Wrapper around the backup which supports getting the connection string among other useful test things
 */
class TestBackup
{
public:
	TestBackup(const boost::filesystem::path& baseDir);
	void Open();
	void Create();
	bslib::Backup& OpenOrCreate();
	std::unique_ptr<bslib::sqlitepp::ScopedSqlite3Object> ConnectToDatabase() const;
	const boost::filesystem::path& GetDirectoryStorePath() const { return _baseDir; }
	const boost::filesystem::path& GetBackupDatabaseDbPath() const { return _backupDatabasePath; }
	bslib::BackupDatabase& GetBackupDatabase();
	bslib::blob::BlobStoreManager& GetBlobStoreManager();
	bslib::Backup& GetBackup();
	void Close();
	const bslib::UTF8String& GetName() const { return _name; }
private:
	const bslib::UTF8String _name;
	boost::filesystem::path _baseDir;
	boost::filesystem::path _backupDatabasePath;
	std::unique_ptr<bslib::Backup> _backup;
	bslib::blob::BlobStoreManager _blobStoreManager;
};

}
}

