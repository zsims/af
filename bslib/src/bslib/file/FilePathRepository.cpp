#include "bslib/file/FilePathRepository.hpp"

#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

namespace af {
namespace bslib {
namespace file {

namespace {
enum FilePathColumnIndex
{
	FilePath_ColumnIndex_Id = 0,
	FilePath_ColumnIndex_FullPath,
	FilePath_ColumnIndex_FileType,
	FilePath_ColumnIndex_ParentId
};

boost::optional<int64_t> FindCachedPath(FilePathRepository::cache_type& cache, const fs::NativePath& path, FileType type)
{
	const auto it = cache.find(path);
	if (it != cache.end())
	{
		const auto typeIt = it->second.find(type);
		if (typeIt != it->second.end())
		{
			return typeIt->second;
		}
	}

	return boost::none;
}

void AddCachedPath(FilePathRepository::cache_type& cache, const fs::NativePath& path, int64_t pathId, FileType type)
{
	const auto it = cache.find(path);
	if (it == cache.end())
	{
		std::unordered_map<FileType, int64_t> m = { {type, pathId} };
		cache.insert(std::make_pair(path, m));
		return;
	}

	it->second.insert(std::make_pair(type, pathId));
}

}

FilePathRepository::FilePathRepository(const sqlitepp::ScopedSqlite3Object& connection)
	: _db(connection)
{
}

std::vector<std::pair<int64_t, fs::NativePath>> FilePathRepository::GetAllPaths() const
{
	const auto query = "SELECT Id, FullPath FROM FilePath";
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query, statement);
	std::vector<std::pair<int64_t, fs::NativePath>> result;
	auto stepResult = 0;
	while ((stepResult = sqlite3_step(statement)) == SQLITE_ROW)
	{
		const auto id = sqlite3_column_int64(statement, FilePath_ColumnIndex_Id);
		const auto rawFullPath = sqlite3_column_text(statement, FilePath_ColumnIndex_FullPath);
		const fs::NativePath fullPath(reinterpret_cast<const char*>(rawFullPath));
		result.push_back(std::make_pair(id, fullPath));
	}

	return result;
}

int64_t FilePathRepository::AddPath(const fs::NativePath& path, FileType type, const boost::optional<int64_t>& parentId)
{
	const auto query = "INSERT INTO FilePath (FullPath, FileType, ParentId) VALUES(:FullPath, :FileType, :ParentId)";
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query, statement);
	const auto& rawPath = path.ToString();
	sqlitepp::BindByParameterNameText(statement, ":FullPath", rawPath);
	sqlitepp::BindByParameterNameInt32(statement, ":FileType", static_cast<int>(type));

	if (parentId)
	{
		sqlitepp::BindByParameterNameInt64(statement, ":ParentId", parentId.value());
	}
	else
	{
		sqlitepp::BindByParameterNameNull(statement, ":ParentId");
	}

	const auto stepResult = sqlite3_step(statement);
	if (stepResult != SQLITE_DONE)
	{
		throw AddFilePathFailedException(stepResult);
	}

	return sqlite3_last_insert_rowid(_db);
}

int64_t FilePathRepository::AddPathTree(const fs::NativePath& path, FileType type, cache_type& lookupCache)
{
	const auto pathSegments = path.GetIntermediatePaths();
	boost::optional<int64_t> lastSegmentId;
	const auto size = pathSegments.size();
	for (auto i = 0U; i < size; i++)
	{
		// Assume any parents are directories
		auto segmentType = FileType::Directory;

		// on the last segment, use the provided type
		if (i == (size - 1))
		{
			segmentType = type;
		}

		const auto& segment = pathSegments[i];
		int64_t segmentId;

		// Don't bother looking it up if we already know about it
		auto cachedPath = FindCachedPath(lookupCache, segment, segmentType);
		if (cachedPath)
		{
			segmentId = cachedPath.value();
		}
		else
		{
			// Find in DB or add if it doesn't exist
			const auto existingId = FindPath(segment, segmentType);
			if (!existingId)
			{
				segmentId = AddPath(segment, segmentType, lastSegmentId);
			}
			else
			{
				segmentId = existingId.value();
			}

			AddCachedPath(lookupCache, segment, segmentId, segmentType);
		}
		lastSegmentId = segmentId;
	}

	return lastSegmentId.value();
}

int64_t FilePathRepository::AddPathTree(const fs::NativePath& path, FileType type)
{
	cache_type knownPaths;
	return AddPathTree(path, type, knownPaths);
}

boost::optional<int64_t> FilePathRepository::FindPath(const fs::NativePath& path, FileType type) const
{
	const auto query = "SELECT Id FROM FilePath WHERE FullPath = :FullPath AND FileType = :FileType";
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query, statement);
	const auto& rawPath = path.ToString();
	sqlitepp::BindByParameterNameText(statement, ":FullPath", rawPath);
	sqlitepp::BindByParameterNameInt32(statement, ":FileType", static_cast<int>(type));
	const auto stepResult = sqlite3_step(statement);
	if(stepResult == SQLITE_ROW)
	{
		return sqlite3_column_int64(statement, FilePath_ColumnIndex_Id);
	}

	return boost::none;
}

boost::optional<StoredPath> FilePathRepository::FindPathDetails(const fs::NativePath& path, FileType type) const
{
	const auto query = "SELECT Id, NULL, FileType, ParentId FROM FilePath WHERE FullPath = :FullPath AND FileType = :FileType";
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query, statement);
	const auto& rawPath = path.ToString();
	sqlitepp::BindByParameterNameText(statement, ":FullPath", rawPath);
	sqlitepp::BindByParameterNameInt32(statement, ":FileType", static_cast<int>(type));
	const auto stepResult = sqlite3_step(statement);
	if(stepResult == SQLITE_ROW)
	{
		boost::optional<int64_t> parentId;
		if (sqlite3_column_type(statement, FilePath_ColumnIndex_ParentId) != SQLITE_NULL)
		{
			parentId = sqlite3_column_int64(statement, FilePath_ColumnIndex_ParentId);
		}
		return StoredPath(sqlite3_column_int64(statement, FilePath_ColumnIndex_Id), type, parentId);
	}

	return boost::none;
}

}
}
}