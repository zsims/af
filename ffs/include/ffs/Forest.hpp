#pragma once

#include "ffs/Address.hpp"
#include "ffs/object/ObjectInfo.hpp"

#include <memory>
#include <string>
#include <random>

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
	 * Creates a new object.
	 */
	ObjectAddress CreateObject(const std::string& type, const object::ObjectBlobList& objectBlobs);

	/**
	 * Gets an object by address.
	 * \throws ObjectNotFoundException No object with the given address could be found.
	 */
	object::ObjectInfo GetObject(const ObjectAddress& address) const;

	/**
	 * Creates a new blob.
	 */
	BlobAddress CreateBlob(const std::vector<uint8_t>& content);

	/**
	 * Gets a blob by address.
	 * \exception BlobReadException The blob with the given address couldn't be read, e.g. it doesn't exist or a permissions failure.
	 */
	std::vector<uint8_t> GetBlob(const BlobAddress& address);
private:
	const std::string _utf8DbPath;
	std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
	std::shared_ptr<blob::BlobStore> _blobStore;
	std::shared_ptr<blob::BlobInfoRepository> _blobInfoRepository;
	std::shared_ptr<object::ObjectInfoRepository> _objectInfoRepository;
	std::mt19937 _random;
};

}
}