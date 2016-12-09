#pragma once

#include "bslib/file/fs/path.hpp"
#include "bslib/sqlitepp/handles.hpp"

#include <boost/optional.hpp>

#include <vector>
#include <unordered_map>

namespace af {
namespace bslib {
namespace file {

struct StoredPath
{
	StoredPath(int64_t pathId, const boost::optional<int64_t> parentId)
		: pathId(pathId)
		, parentId(parentId)
	{
	}
	const int64_t pathId;
	const boost::optional<int64_t> parentId;
};

class FilePathRepository
{
public:
	explicit FilePathRepository(const sqlitepp::ScopedSqlite3Object& connection);
	std::vector<std::pair<int64_t, fs::NativePath>> GetAllPaths() const;

	int64_t AddPath(const fs::NativePath& path, const boost::optional<int64_t>& parentId);

	/**
	 * Adds the full path tree for the given path. E.g. given C:\foo\bar, then C:\ will be added (if it doesn't exist), then C:\foo\ with a parent of C:\, etc
	 */
	int64_t AddPathTree(const fs::NativePath& path, std::unordered_map<fs::NativePath, int64_t>& lookupCache);
	int64_t AddPathTree(const fs::NativePath& path);

	boost::optional<int64_t> FindPath(const fs::NativePath& path) const;
	boost::optional<StoredPath> FindPathDetails(const fs::NativePath& path) const;
private:
	const sqlitepp::ScopedSqlite3Object& _db;
};

}
}
}