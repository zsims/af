#include "bslib/file/FileBackupRunEventStreamRepository.hpp"

#include "bslib/file/FileBackupRunEvent.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/format.hpp>
#include <sqlite3.h>

#include <memory>

namespace af {
namespace bslib {
namespace file {

namespace {
enum GetObjectColumnIndex
{
	GetFileBackupRunEvent_ColumnIndex_Id = 0,
	GetFileBackupRunEvent_ColumnIndex_DateUtc,
	GetFileBackupRunEvent_ColumnIndex_BackupRunId,
	GetFileBackupRunEvent_ColumnIndex_Action
};
}

FileBackupRunEventStreamRepository::FileBackupRunEventStreamRepository(const sqlitepp::ScopedSqlite3Object& connection)
	: _db(connection)
{
	sqlitepp::prepare_or_throw(_db, R"(
		INSERT INTO FileBackupRunEvent (DateTimeUtc, BackupRunId, Action) VALUES (:DateTimeUtc, :BackupRunId, :Action)
	)", _insertEventStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT Id, DateTimeUtc, BackupRunId, Action FROM FileBackupRunEvent
		ORDER BY Id ASC
	)", _getAllEventsStatement);
}

std::vector<FileBackupRunEvent> FileBackupRunEventStreamRepository::GetAllEvents() const
{
	std::vector<FileBackupRunEvent> result;
	sqlitepp::ScopedStatementReset reset(_getAllEventsStatement);

	auto stepResult = 0;
	while ((stepResult = sqlite3_step(_getAllEventsStatement)) == SQLITE_ROW)
	{
		result.push_back(MapRowToEvent(_getAllEventsStatement));
	}

	return result;
}

void FileBackupRunEventStreamRepository::AddEvent(const FileBackupRunEvent& backupEvent)
{
	static boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
	const auto secs = (backupEvent.dateTimeUtc - epoch).total_seconds();
	sqlitepp::ScopedStatementReset reset(_insertEventStatement);
	sqlitepp::BindByParameterNameInt64(_insertEventStatement, ":DateTimeUtc", secs);
	sqlitepp::BindByParameterNameInt64(_insertEventStatement, ":Action", static_cast<int64_t>(backupEvent.action));

	// This needs to stay in scope until the step
	auto byteUuid = backupEvent.runId.ToArray();
	sqlitepp::BindByParameterNameBlob(_insertEventStatement, ":BackupRunId", &byteUuid[0], byteUuid.size());

	const auto stepResult = sqlite3_step(_insertEventStatement);
	if (stepResult != SQLITE_DONE)
	{
		throw AddFileBackupRunEventFailedException(stepResult);
	}
}

FileBackupRunEvent FileBackupRunEventStreamRepository::MapRowToEvent(const sqlitepp::ScopedStatement& statement) const
{
	const auto runIdBytesCount = sqlite3_column_bytes(statement, GetFileBackupRunEvent_ColumnIndex_BackupRunId);
	const auto runIdBytes = sqlite3_column_blob(statement, GetFileBackupRunEvent_ColumnIndex_BackupRunId);
	const Uuid runId(runIdBytes, runIdBytesCount);
	const FileBackupRunEventAction action = static_cast<FileBackupRunEventAction>(sqlite3_column_int(statement, GetFileBackupRunEvent_ColumnIndex_Action));
	const auto unixDate = static_cast<time_t>(sqlite3_column_int(statement, GetFileBackupRunEvent_ColumnIndex_DateUtc));
	const auto pt = boost::posix_time::from_time_t(unixDate);
	return FileBackupRunEvent(runId, pt, action);
}

}
}
}