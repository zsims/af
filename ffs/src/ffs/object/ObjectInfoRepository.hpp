#pragma once

#include "ffs/Address.hpp"
#include "ffs/object/ObjectInfo.hpp"

#include <cstdint>
#include <memory>
#include <vector>

#include "ffs/sqlite/handles.hpp"

namespace af {
namespace ffs {
namespace object {

/**
 * Maintains object information
 */
class ObjectInfoRepository
{
public:
	/**
	 * Creates a new object info repository given an existing database path.
	 */
	explicit ObjectInfoRepository(const std::string& utf8DbPath);

	std::vector<ObjectInfoPtr> GetAllObjects() const;

	void AddObject(const ObjectInfo& info);
	ObjectInfo GetObject(const ObjectAddress& address) const;
private:
	void PrepareInsertObjectStatement();
	void PrepareInsertObjectBlobStatement();
	void PrepareGetObjectStatement();
	void PrepareGetAllObjectsStatement();
	void InsertObjectBlobs(const binary_address& objectAddress, const ObjectBlobList& objectBlobs);

	sqlite::ScopedSqlite3Object _db;
	sqlite::ScopedStatement _insertObjectStatement;
	sqlite::ScopedStatement _insertObjectBlobStatement;
	sqlite::ScopedStatement _getObjectStatement;
	sqlite::ScopedStatement _getAllObjectsStatement;
};

typedef std::shared_ptr<ObjectInfoRepository> ObjectInfoRepositoryPtr;

}
}
}