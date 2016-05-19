#pragma once

#include "ffs/Address.hpp"
#include "ffs/object/ObjectInfo.hpp"

#include <cstdint>
#include <memory>
#include <vector>

#include "ffs/sqlitepp/handles.hpp"

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

	sqlitepp::ScopedSqlite3Object _db;
	sqlitepp::ScopedStatement _insertObjectStatement;
	sqlitepp::ScopedStatement _insertObjectBlobStatement;
	sqlitepp::ScopedStatement _getObjectStatement;
	sqlitepp::ScopedStatement _getAllObjectsStatement;
};

typedef std::shared_ptr<ObjectInfoRepository> ObjectInfoRepositoryPtr;

}
}
}