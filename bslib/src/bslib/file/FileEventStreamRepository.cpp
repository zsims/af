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
	if (criteria.ancestorPath)
	{
		if (and)
		{
			ss << " AND ";
		}
		ss << "FileEvent.PathId IN (SELECT PathId FROM FilePathParent WHERE ParentPathId = (SELECT Id FROM FilePath WHERE FullPath = :Needle LIMIT 1))";
	}
	return ss.str();
}

}

FileEventStreamRepository::FileEventStreamRepository(const sqlitepp::ScopedSqlite3Object& connection)
	: _db(connection)
{
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT FileEvent.Id, FilePath.FullPath, FileEvent.ContentBlobAddress, FileEvent.Action, FileEvent.FileType, FileEvent.BackupRunId FROM FileEvent
		JOIN FilePath ON FileEvent.PathId = FilePath.Id
		ORDER BY FileEvent.Id ASC
	)", _getAllEventsStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT FileEvent.Id, FilePath.FullPath, FileEvent.ContentBlobAddress, FileEvent.Action, FileEvent.FileType, FileEvent.BackupRunId FROM FileEvent
		JOIN FilePath ON FileEvent.PathId = FilePath.Id
		WHERE FilePath.Id IN (
			SELECT PathId FROM FilePathParent WHERE ParentPathId = (SELECT Id FROM FilePath WHERE FullPath = :Needle LIMIT 1)
		) AND FileEvent.Action IN (0, 1, 2)
		GROUP BY FileEvent.PathId HAVING FileEvent.Id = MAX(FileEvent.Id)
	)", _getLastChangedEventsUnderPathStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT FileEvent.Id, FilePath.FullPath, FileEvent.ContentBlobAddress, FileEvent.Action, FileEvent.FileType, FileEvent.BackupRunId FROM FileEvent
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

std::vector<FileEvent> FileEventStreamRepository::SearchDistinctPath(
	const FileEventSearchCriteria& criteria,
	const std::set<FileEventAction>& reducedActions,
	unsigned skip,
	unsigned limit) const
{
	std::stringstream queryss;
	queryss << R"(
		SELECT FileEvent.Id, FilePath.FullPath, FileEvent.ContentBlobAddress, FileEvent.Action, FileEvent.FileType, FileEvent.BackupRunId 
		FROM FileEvent
		JOIN FilePath ON FileEvent.PathId = FilePath.Id
		WHERE FileEvent.Id IN (
			SELECT MAX(FileEvent.Id)
			FROM FileEvent
			WHERE )";
	queryss << BuildPredicate(criteria);
	queryss << "GROUP BY FileEvent.PathId)";
	if (!reducedActions.empty())
	{
		const auto reducedActionsSet = sqlitepp::ToSetLiteral(reducedActions, [](const FileEventAction& a) {
			return std::to_string(static_cast<int>(a));
		});
		queryss << " AND FileEvent.Action IN " << reducedActionsSet;
	}
	queryss << " ORDER BY FileEvent.Id DESC LIMIT :Skip, :Limitation";
	const auto query = queryss.str();

	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query.c_str(), statement);

	// bind needle but make sure it stays in scope for the query
	std::string needle;
	if (criteria.ancestorPath)
	{
		needle = criteria.ancestorPath.value();
		sqlitepp::BindByParameterNameText(statement, ":Needle", needle);
	}
	sqlitepp::BindByParameterNameInt64(statement, ":Skip", static_cast<int64_t>(skip));
	sqlitepp::BindByParameterNameInt64(statement, ":Limitation", static_cast<int64_t>(limit));
	std::vector<FileEvent> result;
	auto stepResult = 0;
	while ((stepResult = sqlite3_step(statement)) == SQLITE_ROW)
	{
		result.push_back(MapRowToEvent(statement));
	}
	return result;
}

std::vector<FileEvent> FileEventStreamRepository::SearchDistinctPath(const FileEventSearchCriteria& criteria, unsigned skip, unsigned limit) const
{
	return SearchDistinctPath(criteria, {}, skip, limit);
}

std::vector<FileEvent> FileEventStreamRepository::Search(const FileEventSearchCriteria& criteria, unsigned skip, unsigned limit) const
{
	const auto predicate = BuildPredicate(criteria);
	std::stringstream queryss;
	queryss << "SELECT FileEvent.Id, FilePath.FullPath, FileEvent.ContentBlobAddress, FileEvent.Action, FileEvent.FileType, FileEvent.BackupRunId FROM FileEvent ";
	queryss << "JOIN FilePath ON FileEvent.PathId = FilePath.Id";
	if(!predicate.empty())
	{
		queryss << " WHERE " << predicate;
	}
	queryss << " ORDER BY FileEvent.Id ASC";
	queryss << " LIMIT " << skip << ", " << limit;
	const auto query = queryss.str();
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query.c_str(), statement);
	std::string needle;
	if (criteria.ancestorPath)
	{
		needle = criteria.ancestorPath.value();
		sqlitepp::BindByParameterNameText(statement, ":Needle", needle);
	}
	std::vector<FileEvent> result;
	auto stepResult = 0;
	while ((stepResult = sqlite3_step(statement)) == SQLITE_ROW)
	{
		result.push_back(MapRowToEvent(statement));
	}
	return result;
}

unsigned FileEventStreamRepository::CountMatching(const FileEventSearchCriteria& criteria) const
{
	const auto predicate = BuildPredicate(criteria);
	std::stringstream queryss;
	queryss << "SELECT COUNT(*) FROM FileEvent ";
	queryss << "JOIN FilePath ON FileEvent.PathId = FilePath.Id";
	if(!predicate.empty())
	{
		queryss << " WHERE " << predicate;
	}
	const auto query = queryss.str();
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query.c_str(), statement);
	std::string needle;
	if (criteria.ancestorPath)
	{
		needle = criteria.ancestorPath.value();
		sqlitepp::BindByParameterNameText(statement, ":Needle", needle);
	}
	const auto stepResult = sqlite3_step(statement);
	if(stepResult == SQLITE_ROW)
	{
		return static_cast<unsigned>(sqlite3_column_int64(statement, 0));
	}
	return 0;
}

unsigned FileEventStreamRepository::CountMatchingDistinctPath(const FileEventSearchCriteria& criteria) const
{
	const auto predicate = BuildPredicate(criteria);
	std::stringstream queryss;
	queryss << "SELECT COUNT(*) FROM (SELECT FileEvent.Id FROM FileEvent ";
	queryss << "JOIN FilePath ON FileEvent.PathId = FilePath.Id";
	if(!predicate.empty())
	{
		queryss << " WHERE " << predicate;
	}
	queryss << " GROUP BY FileEvent.PathId)";
	const auto query = queryss.str();
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query.c_str(), statement);
	std::string needle;
	if (criteria.ancestorPath)
	{
		needle = criteria.ancestorPath.value();
		sqlitepp::BindByParameterNameText(statement, ":Needle", needle);
	}
	const auto stepResult = sqlite3_step(statement);
	if(stepResult == SQLITE_ROW)
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