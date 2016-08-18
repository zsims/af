#pragma once

#include "bslib/Address.hpp"
#include "bslib/file/FileRef.hpp"
#include "bslib/sqlitepp/handles.hpp"

#include <boost/optional.hpp>

#include <cstdint>
#include <memory>
#include <vector>


namespace af {
namespace bslib {
namespace file {

/**
 * Maintains file references to file objects
 */
class FileRefRepository
{
public:
	explicit FileRefRepository(const sqlitepp::ScopedSqlite3Object& connection);

	void SetReference(const FileRef& reference);
	FileRef GetReference(const std::string& fullPath) const;
	boost::optional<FileRef> FindReference(const std::string& fullPath) const;
private:
	const sqlitepp::ScopedSqlite3Object& _db;
	sqlitepp::ScopedStatement _insertRefStatement;
	sqlitepp::ScopedStatement _getRefStatement;
};

}
}
}