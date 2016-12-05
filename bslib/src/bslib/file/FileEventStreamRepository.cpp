#include "bslib/file/FileEventStreamRepository.hpp"

#include "bslib/blob/Address.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileEvent.hpp"
#include "bslib/file/fs/path.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>
#include <sqlite3.h>

#include <memory>
#include <sstream>
#include <utility>

namespace af {
namespace bslib {
namespace file {

namespace {
enum GetObjectColumnIndex
{
	GetFileEvent_ColumnIndex_Id = 0,
	GetFileEvent_ColumnIndex_PathId,
	GetFileEvent_ColumnIndex_FullPath,
	GetFileEvent_ColumnIndex_ContentBlobAddress,
	GetFileEvent_ColumnIndex_Action,
	GetFileEvent_ColumnIndex_FileType,
	GetFileEvent_ColumnIndex_BackupRunId
};

std::string BuildPredicate(const FileEventSearchCriteria& criteria)
{
	std::stringstream ss;
	bool and = false;
	if (criteria.runId)
	{
		ss << "FileEvent.BackupRunId = X'" << criteria.runId->ToDashlessString() << "'";
		and = true;
	}
	if (!criteria.actions.empty())
	{
		if (and)
		{
			ss << " AND ";
		}
		ss << "FileEvent.Action IN " << sqlitepp::ToSetLiteral(criteria.actions, [](const FileEventAction& a) { return std::to_string(static_cast<int>(a)); });
		and = true;
	}
	return ss.str();
}

std::string BuildPredicate(const FilePathSearchCriteria& criteria)
{
	std::stringstream ss;
	bool and = false;
	if (criteria.parentPathId)
	{
		ss << "FilePath.Id = " << criteria.parentPathId.value();
	}
	return ss.str();
}

}

FileEventStreamRepository::FileEventStreamRepository(const sqlitepp::ScopedSqlite3Object& connection)
	: _db(connection)
{
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT FileEvent.Id, FilePath.Id, FilePath.FullPath, FileEvent.ContentBlobAddress, FileEvent.Action, FileEvent.FileType, FileEvent.BackupRunId FROM FileEvent
		JOIN FilePath ON FileEvent.PathId = FilePath.Id
		ORDER BY FileEvent.Id ASC
	)", _getAllEventsStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		WITH RECURSIVE path_decedent(n) AS (
			SELECT Id FROM FilePath WHERE FullPath = :Needle
			UNION ALL
			SELECT Id From FilePath, path_decedent WHERE FilePath.ParentId = path_decedent.n
		)
		SELECT FileEvent.Id, FilePath.Id, FilePath.FullPath, FileEvent.ContentBlobAddress, FileEvent.Action, FileEvent.FileType, FileEvent.BackupRunId FROM FileEvent
		JOIN FilePath ON FileEvent.PathId = FilePath.Id
		WHERE FilePath.Id IN path_decedent AND FileEvent.Action IN (0, 1, 2)
		GROUP BY FileEvent.PathId HAVING FileEvent.Id = MAX(FileEvent.Id)
	)", _getLastChangedEventsUnderPathStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT FileEvent.Id, FilePath.Id, FilePath.FullPath, FileEvent.ContentBlobAddress, FileEvent.Action, FileEvent.FileType, FileEvent.BackupRunId FROM FileEvent
		JOIN FilePath ON FileEvent.PathId = FilePath.Id
		WHERE FilePath.FullPath = :FullPath AND FileEvent.Action IN (0, 1, 2)
		ORDER BY FileEvent.Id DESC LIMIT 1
	)", _getLastChangedEventByPathStatement);
}

std::vector<FileEvent> FileEventStreamRepository::GetAllEvents() const
{
	std::vector<FileEvent> result;
	sqlitepp::ScopedStatementReset reset(_getAllEventsStatement);

	auto stepResult = 0;
	while ((stepResult = sqlite3_step(_getAllEventsStatement)) == SQLITE_ROW)
	{
		result.push_back(MapRowToEvent(_getAllEventsStatement));
	}

	return result;
}

std::map<fs::NativePath, FileEvent> FileEventStreamRepository::GetLastChangedEventsUnderPath(const fs::NativePath& fullPath) const
{
	std::map<fs::NativePath, FileEvent> result;
	const auto needle = fullPath.ToString();

	sqlitepp::ScopedStatementReset reset(_getLastChangedEventsUnderPathStatement);
	sqlitepp::BindByParameterNameText(_getLastChangedEventsUnderPathStatement, ":Needle", needle);

	auto stepResult = 0;
	while ((stepResult = sqlite3_step(_getLastChangedEventsUnderPathStatement)) == SQLITE_ROW)
	{
		const auto& row = MapRowToEvent(_getLastChangedEventsUnderPathStatement);
		result.insert(std::make_pair(row.fullPath, row));
	}

	return result;
}

void FileEventStreamRepository::AddEvent(const FileEvent& fileEvent, int64_t pathId)
{
	const auto query = "INSERT INTO FileEvent (PathId, ContentBlobAddress, Action, FileType, BackupRunId) VALUES (:PathId, :ContentBlobAddress, :Action, :FileType, :BackupRunId)";
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query, statement);
	sqlitepp::BindByParameterNameInt64(statement, ":PathId", pathId);

	// TODO: This has to stay in scope for the duration of the statement, find a better way to do this without copying
	blob::binary_address binaryContentAddress;

	if (fileEvent.contentBlobAddress)
	{
		binaryContentAddress = fileEvent.contentBlobAddress.value().ToBinary();
		sqlitepp::BindByParameterNameBlob(statement, ":ContentBlobAddress", &binaryContentAddress[0], binaryContentAddress.size());
	}
	else
	{
		sqlitepp::BindByParameterNameNull(statement, ":ContentBlobAddress");
	}

	auto byteUuid = fileEvent.backupRunId.ToArray();
	sqlitepp::BindByParameterNameBlob(statement, ":BackupRunId", &byteUuid[0], byteUuid.size());
	sqlitepp::BindByParameterNameInt64(statement, ":Action", static_cast<int64_t>(fileEvent.action));
	sqlitepp::BindByParameterNameInt64(statement, ":FileType", static_cast<int64_t>(fileEvent.type));

	const auto stepResult = sqlite3_step(statement);
	if (stepResult != SQLITE_DONE)
	{
		throw AddFileEventFailedException((boost::format("Failed to execute statement for insert event. SQLite error %1%") % stepResult).str());
	}
}

boost::optional<FileEvent> FileEventStreamRepository::FindLastChangedEvent(const fs::NativePath& fullPath) const
{
	sqlitepp::ScopedStatementReset reset(_getLastChangedEventByPathStatement);
	const auto rawPath = fullPath.ToString();
	sqlitepp::BindByParameterNameText(_getLastChangedEventByPathStatement, ":FullPath", rawPath);

	auto stepResult = sqlite3_step(_getLastChangedEventByPathStatement);
	if (stepResult != SQLITE_ROW)
	{
		return boost::none;
	}

	return MapRowToEvent(_getLastChangedEventByPathStatement);
}

std::map<Uuid, FileEventStreamRepository::RunStats> FileEventStreamRepository::GetStatisticsByRunId(
	const std::vector<Uuid>& runIds,
	const std::set<FileEventAction>& actions) const
{
	// Convert UUIDS to a set of hex literals, e.g. (X'000000..', X'123')
	const auto idsSet = sqlitepp::ToSetLiteral(runIds, [](const Uuid& e) {
		return "X'" + e.ToDashlessString() + "'";
	});

	const auto actionsSet = sqlitepp::ToSetLiteral(actions, [](const FileEventAction& a) {
		return std::to_string(static_cast<int>(a));
	});

	const std::string query(R"(
		SELECT FileEvent.BackupRunId, COUNT(FileEvent.Id), SUM(Blob.SizeBytes) FROM FileEvent
		LEFT OUTER JOIN Blob ON FileEvent.ContentBlobAddress = Blob.Address
		WHERE FileEvent.BackupRunId IN )" + idsSet + " AND FileEvent.Action IN " + actionsSet + R"(
		GROUP BY FileEvent.BackupRunId
	)");
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query.c_str(), statement);
	std::map<Uuid, RunStats> result;
	auto stepResult = 0;
	while ((stepResult = sqlite3_step(statement)) == SQLITE_ROW)
	{
		const auto runIdBytesCount = sqlite3_column_bytes(statement, 0);
		const auto runIdBytes = sqlite3_column_blob(statement, 0);
		const Uuid runId(runIdBytes, runIdBytesCount);
		const auto count = sqlite3_column_int64(statement, 1);
		const auto totalSize = sqlite3_column_int64(statement, 2);
		RunStats stats;
		stats.matchingEvents = static_cast<unsigned>(count);
		stats.matchingSizeBytes = static_cast<uint64_t>(totalSize);
		result.insert(std::make_pair(runId, stats));
	}

	// Fill out "no match" for remaining
	for (const auto& runId : runIds)
	{
		auto it = result.find(runId);
		if (it == result.end())
		{
			result.insert(std::make_pair(runId, RunStats()));
		}
	}

	return result;
}

std::vector<FileEventStreamRepository::PathFirstSearchMatch> FileEventStreamRepository::SearchPathFirst(
	const FilePathSearchCriteria& pathCriteria,
	const FileEventSearchCriteria& eventCriteria,
	unsigned skip,
	unsigned limit) const
{
	std::stringstream queryss;
	queryss << R"(
		SELECT FileEvent.Id, FilePath.Id, FilePath.FullPath, FileEvent.ContentBlobAddress, FileEvent.Action, FileEvent.FileType, FileEvent.BackupRunId 
		FROM FilePath
		LEFT OUTER JOIN FileEvent ON FileEvent.PathId = FilePath.Id AND FileEvent.Id IN (
			SELECT MAX(FileEvent.Id)
			FROM FileEvent)";
	const auto eventPredicate = BuildPredicate(eventCriteria);
	if (!eventPredicate.empty())
	{
		queryss << " WHERE " << eventPredicate;
	}
	queryss << " GROUP BY FileEvent.PathId)";
	const auto pathPredicate = BuildPredicate(pathCriteria);
	if (!pathPredicate.empty())
	{
		queryss << " WHERE " << pathPredicate;
	}
	queryss << " ORDER BY FilePath.Id DESC LIMIT :Skip, :Limitation";
	const auto query = queryss.str();

	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query.c_str(), statement);

	// bind needle but make sure it stays in scope for the query
	sqlitepp::BindByParameterNameInt64(statement, ":Skip", static_cast<int64_t>(skip));
	sqlitepp::BindByParameterNameInt64(statement, ":Limitation", static_cast<int64_t>(limit));
	std::vector<PathFirstSearchMatch> result;
	auto stepResult = 0;
	while ((stepResult = sqlite3_step(statement)) == SQLITE_ROW)
	{
		boost::optional<FileEvent> matchedEvent;
		if (sqlite3_column_type(statement, GetFileEvent_ColumnIndex_Id) != SQLITE_NULL)
		{
			matchedEvent = MapRowToEvent(statement);
		}
		const auto rawFullPath = sqlite3_column_text(statement, GetFileEvent_ColumnIndex_FullPath);
		PathFirstSearchMatch match;
		match.latestEvent = matchedEvent;
		match.fullPath =  fs::NativePath(reinterpret_cast<const char*>(rawFullPath));
		match.pathId = sqlite3_column_int64(statement, GetFileEvent_ColumnIndex_PathId);
		result.push_back(match);
	}
	return result;
}

std::vector<FileEvent> FileEventStreamRepository::Search(const FilePathSearchCriteria& pathCriteria, const FileEventSearchCriteria& eventCriteria, unsigned skip, unsigned limit) const
{
	std::stringstream queryss;
	queryss << "SELECT FileEvent.Id, FilePath.Id, FilePath.FullPath, FileEvent.ContentBlobAddress, FileEvent.Action, FileEvent.FileType, FileEvent.BackupRunId FROM FileEvent ";
	queryss << "JOIN FilePath ON FileEvent.PathId = FilePath.Id";
	const auto eventPredicate = BuildPredicate(eventCriteria);
	if (!eventPredicate.empty())
	{
		queryss << " WHERE " << eventPredicate;
	}
	const auto pathPredicate = BuildPredicate(pathCriteria);
	if (!pathPredicate.empty())
	{
		if (!eventPredicate.empty())
		{
			queryss << " AND ";
		}
		else
		{
			queryss << " WHERE ";
		}
		queryss << pathPredicate;
	}
	queryss << " ORDER BY FileEvent.Id ASC";
	queryss << " LIMIT " << skip << ", " << limit;
	const auto query = queryss.str();
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query.c_str(), statement);
	std::vector<FileEvent> result;
	auto stepResult = 0;
	while ((stepResult = sqlite3_step(statement)) == SQLITE_ROW)
	{
		result.push_back(MapRowToEvent(statement));
	}
	return result;
}

std::vector<FileEvent> FileEventStreamRepository::Search(const FileEventSearchCriteria& eventCriteria, unsigned skip, unsigned limit) const
{
	return Search(FilePathSearchCriteria{}, eventCriteria, skip, limit);
}

unsigned FileEventStreamRepository::CountMatching(const FilePathSearchCriteria& pathCriteria, const FileEventSearchCriteria& eventCriteria) const
{
	std::stringstream queryss;
	queryss << "SELECT COUNT(*) FROM FileEvent ";
	queryss << "JOIN FilePath ON FileEvent.PathId = FilePath.Id";
	const auto eventPredicate = BuildPredicate(eventCriteria);
	if(!eventPredicate.empty())
	{
		queryss << " WHERE " << eventPredicate;
	}
	const auto pathPredicate = BuildPredicate(pathCriteria);
	if(!pathPredicate.empty())
	{
		if (!eventPredicate.empty())
		{
			queryss << " AND ";
		}
		else
		{
			queryss << " WHERE ";
		}
		queryss << pathPredicate;
	}
	const auto query = queryss.str();
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query.c_str(), statement);
	const auto stepResult = sqlite3_step(statement);
	if(stepResult == SQLITE_ROW)
	{
		return static_cast<unsigned>(sqlite3_column_int64(statement, 0));
	}
	return 0;
}

unsigned FileEventStreamRepository::CountMatching(const FileEventSearchCriteria& eventCriteria) const
{
	return CountMatching(FilePathSearchCriteria{}, eventCriteria);
}

unsigned FileEventStreamRepository::CountMatching(const FilePathSearchCriteria& criteria) const
{
	const auto predicate = BuildPredicate(criteria);
	std::stringstream queryss;
	queryss << "SELECT COUNT(*) FROM FilePath ";
	if (!predicate.empty())
	{
		queryss << " WHERE " << predicate;
	}
	const auto query = queryss.str();
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query.c_str(), statement);
	const auto stepResult = sqlite3_step(statement);
	if (stepResult == SQLITE_ROW)
	{
		return static_cast<unsigned>(sqlite3_column_int64(statement, 0));
	}
	return 0;
}

FileEvent FileEventStreamRepository::MapRowToEvent(const sqlitepp::ScopedStatement& statement) const
{
	const auto rawFullPath = sqlite3_column_text(statement, GetFileEvent_ColumnIndex_FullPath);
	const fs::NativePath fullPath(reinterpret_cast<const char*>(rawFullPath));
	
	const auto contentBlobAddressBytesCount = sqlite3_column_bytes(statement, GetFileEvent_ColumnIndex_ContentBlobAddress);
	boost::optional<blob::Address> contentBlobAddress = boost::none;
	if (contentBlobAddressBytesCount > 0)
	{
		const auto contentBlobAddressBytes = sqlite3_column_blob(statement, GetFileEvent_ColumnIndex_ContentBlobAddress);
		contentBlobAddress = blob::Address(contentBlobAddressBytes, contentBlobAddressBytesCount);
	}

	const auto runIdBytesCount = sqlite3_column_bytes(statement, GetFileEvent_ColumnIndex_BackupRunId);
	const auto runIdBytes = sqlite3_column_blob(statement, GetFileEvent_ColumnIndex_BackupRunId);
	const Uuid runId(runIdBytes, runIdBytesCount);

	const FileEventAction action = static_cast<FileEventAction>(sqlite3_column_int(statement, GetFileEvent_ColumnIndex_Action));
	const FileType type = static_cast<FileType>(sqlite3_column_int(statement, GetFileEvent_ColumnIndex_FileType));
	return FileEvent(runId, fullPath, type, contentBlobAddress, action);
}

}
}
}