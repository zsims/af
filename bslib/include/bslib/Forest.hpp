#pragma once

#include "bslib/Address.hpp"
#include "bslib/file/FileObjectInfo.hpp"
#include "bslib/UnitOfWork.hpp"

#include <boost/filesystem/path.hpp>

#include <memory>
#include <string>

namespace af {
namespace bslib {

namespace blob {
class BlobInfoRepository;
class BlobStore;
}

namespace file {
class FileObjectInfoRepository;
}

namespace sqlitepp {
class ScopedSqlite3Object;
}

class Forest
{
public:
	/**
	 * Initializes forest with the given path for opening or creating.
	 */
	explicit Forest(const boost::filesystem::path& forestPath, std::unique_ptr<blob::BlobStore> blobStore);
	~Forest();

	/**
	 * Creates a new unit of work. Note that the forest must remain open while the unit of work is being used.
	 */
	std::unique_ptr<UnitOfWork> CreateUnitOfWork();

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
	std::unique_ptr<blob::BlobStore> _blobStore;
};

}
}