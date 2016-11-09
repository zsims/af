#pragma once

#include "bslib/unicode.hpp"
#include "bslib/UnitOfWork.hpp"

#include <boost/filesystem/path.hpp>

#include <memory>
#include <string>

namespace af {
namespace bslib {

namespace blob {
class BlobStore;
class BlobStoreManager;
}

class BackupDatabase;

/**
 * Provides management of backup metadata and associated blob stores
 */
class Backup
{
public:
	Backup(const boost::filesystem::path& databasePath, const UTF8String& name, const blob::BlobStoreManager& blobStoreManager);
	virtual ~Backup();

	/**
	 * Creates a new unit of work. Note that the backup must remain open while the unit of work is being used.
	 */
	virtual std::unique_ptr<UnitOfWork> CreateUnitOfWork();

	/**
	 * Opens an existing database
	 * \throws DatabaseNotFoundException The database couldn't be found
	 */
	virtual void Open();

	/**
	 * Creates a new database and opens it.
	 * \throws DatabaseAlreadyExistsException A database (or path) already exists at the given path
	 * \throws CreateDatabaseFailedException Couldn't create the database at the given path
	 */
	virtual void Create();

	/**
	 * Opens the database if it exists, otherwise creates a new one and opens that.
	 * \throws DatabaseNotFoundException The database couldn't be found
	 * \throws DatabaseAlreadyExistsException A database (or path) already exists at the given path
	 * \throws CreateDatabaseFailedException Couldn't create the database at the given path
	 */
	virtual void OpenOrCreate();

	/**
	 * Ensures a copy of the database is saved to the current blob stores.
	 * This is safe to call while the backup is being used.
	 */
	virtual void SaveDatabaseCopy();

	// Useful for testing, but not intenteded for public use
	virtual BackupDatabase& GetBackupDatabase() { return *_backupDatabase; }
private:
	const boost::filesystem::path _databasePath;
	const UTF8String _name;
	const blob::BlobStoreManager& _blobStoreManager;
	std::unique_ptr<BackupDatabase> _backupDatabase;
};

}
}