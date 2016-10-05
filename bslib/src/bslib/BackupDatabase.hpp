#pragma once

#include "bslib/UnitOfWork.hpp"

#include <boost/filesystem/path.hpp>

#include <memory>
#include <string>

namespace af {
namespace bslib {

namespace blob {
class BlobInfoRepository;
}

namespace sqlitepp {
class ScopedSqlite3Object;
}

class BackupDatabase
{
public:
	/**
	 * Initializes forest with the given path for opening or creating.
	 */
	explicit BackupDatabase(const boost::filesystem::path& forestPath);
	~BackupDatabase();

	/**
	 * Creates a new unit of work. Note that the forest must remain open while the unit of work is being used.
	 */
	std::unique_ptr<UnitOfWork> CreateUnitOfWork(blob::BlobStore& blobStore);

	/**
	 * Opens an existing forest
	 * \throws DatabaseNotFoundException The forest database couldn't be found
	 */
	void Open();

	/**
	 * Creates a new forest database and opens it.
	 * \throws DatabaseAlreadyExistsException A database (or path) already exists at the given path
	 * \throws CreateDatabaseFailedException Couldn't create the database at the given path
	 */
	void Create();

	/**
	 * Opens the forest if it exists, otherwise creates a new one and opens that.
	 * \throws DatabaseNotFoundException The forest database couldn't be found
	 * \throws DatabaseAlreadyExistsException A database (or path) already exists at the given path
	 * \throws CreateDatabaseFailedException Couldn't create the database at the given path
	 */
	void OpenOrCreate();
private:
	const boost::filesystem::path _forestPath;
	std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
};

}
}