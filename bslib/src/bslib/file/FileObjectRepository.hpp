#pragma once

#include "bslib/Address.hpp"
#include "bslib/file/FileObject.hpp"

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
class FileObjectRepository
{
public:
	/**
	 * Creates a new object info repository given an existing connection.
	 */
	explicit FileObjectRepository(const sqlitepp::ScopedSqlite3Object& connection);

	std::vector<std::shared_ptr<FileObject>> GetAllObjects() const;
	std::vector<std::shared_ptr<FileObject>> GetAllObjectsByParentAddress(const ObjectAddress& parentAddress) const;

	void AddObject(const FileObject& info);
	FileObject GetObject(const ObjectAddress& address) const;
	boost::optional<FileObject> FindObject(const ObjectAddress& address) const;
private:
	std::shared_ptr<FileObject> MapRowToObject(const sqlitepp::ScopedStatement& statement) const;

	const sqlitepp::ScopedSqlite3Object& _db;
	sqlitepp::ScopedStatement _insertObjectStatement;
	sqlitepp::ScopedStatement _getObjectStatement;
	sqlitepp::ScopedStatement _getAllObjectsStatement;
	sqlitepp::ScopedStatement _getAllObjectsByParentStatement;
};

}
}
}