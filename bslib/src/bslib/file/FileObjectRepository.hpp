#pragma once

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
	std::vector<std::shared_ptr<FileObject>> GetAllObjectsByParentId(foid parentId) const;

	foid AddObject(
		const boost::filesystem::path& fullPath,
		const boost::optional<BlobAddress>& contentBlobAddress,
		const boost::optional<foid>& parentId = boost::none);

	FileObject AddGetObject(
		const boost::filesystem::path& fullPath,
		const boost::optional<BlobAddress>& contentBlobAddress,
		const boost::optional<foid>& parentId = boost::none);

	FileObject GetObject(foid id) const;
	boost::optional<FileObject> FindObject(foid id) const;
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