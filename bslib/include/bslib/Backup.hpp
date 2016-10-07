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
}

class BackupDatabase;

/**
 * Determines where the default backup database lives.
 * \remarks The path may not exist
 * \returns The full path to the database path, otherwise an empty path if the default path cannot be determined
 */
boost::filesystem::path GetDefaultBackupDatabasePath();

/**
 * Provides management of backup metadata and associated blob stores
 */
class Backup
{
public:
	Backup(const boost::filesystem::path& databasePath, const UTF8String& name);
	~Backup();

	/**
	 * Creates a new unit of work. Note that the backup must remain open while the unit of work is being used.
	 */
	std::unique_ptr<UnitOfWork> CreateUnitOfWork();

	/**
	 * Adds a new blob store the the backup. This blob store will be used to save blobs plus a copy of the metadata.
	 * \remarks Currently only one blob store is supported
	 */
	void AddBlobStore(std::unique_ptr<blob::BlobStore> blobStore);

	/**
	 * Opens an existing database
	 * \throws DatabaseNotFoundException The database couldn't be found
	 */
	void Open();

	/**
	 * Creates a new database and opens it.
	 * \throws DatabaseAlreadyExistsException A database (or path) already exists at the given path
	 * \throws CreateDatabaseFailedException Couldn't create the database at the given path
	 */
	void Create();

	/**
	 * Opens the database if it exists, otherwise creates a new one and opens that.
	 * \throws DatabaseNotFoundException The database couldn't be found
	 * \throws DatabaseAlreadyExistsException A database (or path) already exists at the given path
	 * \throws CreateDatabaseFailedException Couldn't create the database at the given path
	 */
	void OpenOrCreate();

	// Useful for testing, but not intenteded for public use
	BackupDatabase& GetBackupDatabase() { return *_backupDatabase; }
private:
	const boost::filesystem::path _databasePath;
	const UTF8String _name;
	std::unique_ptr<BackupDatabase> _backupDatabase;
	std::unique_ptr<blob::BlobStore> _blobStore;
};

}
}