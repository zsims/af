#pragma once

#include "bslib/Address.hpp"
#include "bslib/file/FileObjectInfo.hpp"

#include <cstdint>
#include <memory>
#include <vector>

#include "bslib/sqlitepp/handles.hpp"

namespace af {
namespace bslib {
namespace file {

/**
 * Maintains file object information
 */
class FileObjectInfoRepository
{
public:
	/**
	 * Creates a new object info repository given an existing connection.
	 */
	explicit FileObjectInfoRepository(const sqlitepp::ScopedSqlite3Object& connection);

	std::vector<std::shared_ptr<FileObjectInfo>> GetAllObjects() const;

	void AddObject(const FileObjectInfo& info);
	FileObjectInfo GetObject(const ObjectAddress& address) const;
private:
	std::shared_ptr<FileObjectInfo> MapRowToObject(const sqlitepp::ScopedStatement& statement) const;

	const sqlitepp::ScopedSqlite3Object& _db;
	sqlitepp::ScopedStatement _insertObjectStatement;
	sqlitepp::ScopedStatement _getObjectStatement;
	sqlitepp::ScopedStatement _getAllObjectsStatement;
};

}
}
}