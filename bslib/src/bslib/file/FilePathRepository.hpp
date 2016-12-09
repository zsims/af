#pragma once

#include "bslib/file/FileType.hpp"
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
	StoredPath(int64_t pathId, FileType type, const boost::optional<int64_t> parentId)
		: pathId(pathId)
		, type(type)
		, parentId(parentId)
	{
	}
	const int64_t pathId;
	const FileType type;
	const boost::optional<int64_t> parentId;
};

class FilePathRepository
{
public:
	typedef std::unordered_map<fs::NativePath, std::unordered_map<FileType, int64_t>> cache_type;

	explicit FilePathRepository(const sqlitepp::ScopedSqlite3Object& connection);
	std::vector<std::pair<int64_t, fs::NativePath>> GetAllPaths() const;

	int64_t AddPath(const fs::NativePath& path, FileType type, const boost::optional<int64_t>& parentId);

	/**
	 * Adds the full path tree for the given path. E.g. given C:\foo\bar, then C:\ will be added (if it doesn't exist), then C:\foo\ with a parent of C:\, etc
	 */
	int64_t AddPathTree(const fs::NativePath& path, FileType type, cache_type& lookupCache);
	int64_t AddPathTree(const fs::NativePath& path, FileType type);

	boost::optional<int64_t> FindPath(const fs::NativePath& path, FileType type) const;
	boost::optional<StoredPath> FindPathDetails(const fs::NativePath& path, FileType type) const;
private:
	const sqlitepp::ScopedSqlite3Object& _db;
};

}
}
}