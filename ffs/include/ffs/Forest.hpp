#pragma once

#include "ffs/Address.hpp"
#include "ffs/object/ObjectInfo.hpp"
#include "ffs/UnitOfWork.hpp"

#include <memory>
#include <string>

namespace af {
namespace ffs {

namespace blob {
class BlobInfoRepository;
class BlobStore;
}

namespace object {
class ObjectInfoRepository;
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
	explicit Forest(const std::string& utf8DbPath, std::shared_ptr<blob::BlobStore> blobStore);
	~Forest();

	/**
	 * Creates a new unit of work.
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
private:
	const std::string _utf8DbPath;
	std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
	std::shared_ptr<blob::BlobStore> _blobStore;
	std::shared_ptr<blob::BlobInfoRepository> _blobInfoRepository;
	std::shared_ptr<object::ObjectInfoRepository> _objectInfoRepository;
};

}
}