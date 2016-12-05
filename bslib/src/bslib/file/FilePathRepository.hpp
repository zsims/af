#pragma once

#include "bslib/file/fs/path.hpp"
#include "bslib/sqlitepp/handles.hpp"

#include <boost/optional.hpp>

#include <vector>

namespace af {
namespace bslib {
namespace file {

class FilePathRepository
{
public:
	explicit FilePathRepository(const sqlitepp::ScopedSqlite3Object& connection);
	std::vector<std::pair<int64_t, fs::NativePath>> GetAllPaths() const;

	int64_t AddPath(const fs::NativePath& path, const boost::optional<int64_t>& parentId);
	boost::optional<int64_t> FindPath(const fs::NativePath& path) const;
private:
	const sqlitepp::ScopedSqlite3Object& _db;
};

}
}
}