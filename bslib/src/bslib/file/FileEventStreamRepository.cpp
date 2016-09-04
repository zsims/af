#include "bslib/file/FileEventStreamRepository.hpp"

#include "bslib/Address.hpp"
#include "bslib/file/FileEvent.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <sqlite3.h>

#include <memory>
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
	GetFileEvent_ColumnIndex_Action
};
}

FileEventStreamRepository::FileEventStreamRepository(const sqlitepp::ScopedSqlite3Object& connection)
	: _db(connection)
{
	sqlitepp::prepare_or_throw(_db, R"(
		INSERT INTO FileEvent (FullPath, ContentBlobAddress, Action) VALUES (:FullPath, :ContentBlobAddress, :Action)
	)", _insertEventStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT Id, FullPath, ContentBlobAddress, Action FROM FileEvent
		ORDER BY Id ASC
	)", _getAllEventsStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT Id, FullPath, ContentBlobAddress, Action FROM FileEvent
		WHERE FullPath LIKE :Needle ESCAPE '"' AND Action IN (0, 1, 2)
		GROUP BY FullPath HAVING Id = max(Id)
	)", _getLastChangedEventsUnderPathStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT Id, FullPath, ContentBlobAddress, Action FROM FileEvent
		WHERE FullPath = :FullPath AND Action IN (0, 1, 2)
		ORDER BY Id DESC LIMIT 1
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

std::map<boost::filesystem::path, FileEvent> FileEventStreamRepository::GetLastChangedEventsStartingWithPath(const boost::filesystem::path& fullPath) const
{
	std::map<boost::filesystem::path, FileEvent> result;

	// " isn't legal in Linux or Windows paths, so use that as an escape sequence
	// to escape the LIKE operators: http://sqlite.org/lang_expr.html#like
	std::string needle = fullPath.string();
	boost::replace_all(needle, "%", "\"%");
	boost::replace_all(needle, "_", "\"_");
	needle += "%";

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

void FileEventStreamRepository::AddEvent(const FileEvent& fileEvent)
{
	sqlitepp::ScopedStatementReset reset(_insertEventStatement);
	const auto& rawPath = fileEvent.fullPath.string();
	sqlitepp::BindByParameterNameText(_insertEventStatement, ":FullPath", rawPath);

	// TODO: This has to stay in scope for the duration of the statement, find a better way to do this without copying
	binary_address binaryContentAddress;

	if (fileEvent.contentBlobAddress)
	{
		binaryContentAddress = fileEvent.contentBlobAddress.value().ToBinary();
		sqlitepp::BindByParameterNameBlob(_insertEventStatement, ":ContentBlobAddress", &binaryContentAddress[0], binaryContentAddress.size());
	}
	else
	{
		sqlitepp::BindByParameterNameNull(_insertEventStatement, ":ContentBlobAddress");
	}

	sqlitepp::BindByParameterNameInt64(_insertEventStatement, ":Action", static_cast<int64_t>(fileEvent.action));

	const auto stepResult = sqlite3_step(_insertEventStatement);
	if (stepResult != SQLITE_DONE)
	{
		throw AddFileEventFailedException((boost::format("Failed to execute statement for insert event. SQLite error %1%") % stepResult).str());
	}
}

boost::optional<FileEvent> FileEventStreamRepository::FindLastChangedEvent(const boost::filesystem::path& fullPath) const
{
	sqlitepp::ScopedStatementReset reset(_getLastChangedEventByPathStatement);
	const auto& rawPath = fullPath.string();
	sqlitepp::BindByParameterNameText(_getLastChangedEventByPathStatement, ":FullPath", rawPath);

	auto stepResult = sqlite3_step(_getLastChangedEventByPathStatement);
	if (stepResult != SQLITE_ROW)
	{
		return boost::none;
	}

	return MapRowToEvent(_getLastChangedEventByPathStatement);
}

FileEvent FileEventStreamRepository::MapRowToEvent(const sqlitepp::ScopedStatement& statement) const
{
	const auto rawFullPath = sqlite3_column_text(statement, GetFileEvent_ColumnIndex_FullPath);
	const std::string fullPath(reinterpret_cast<const char*>(rawFullPath));
	
	const auto contentBlobAddressBytesCount = sqlite3_column_bytes(statement, GetFileEvent_ColumnIndex_ContentBlobAddress);
	boost::optional<BlobAddress> contentBlobAddress = boost::none;
	if (contentBlobAddressBytesCount > 0)
	{
		const auto contentBlobAddressBytes = sqlite3_column_blob(statement, GetFileEvent_ColumnIndex_ContentBlobAddress);
		contentBlobAddress = BlobAddress(contentBlobAddressBytes, contentBlobAddressBytesCount);
	}

	const FileEventAction action = static_cast<FileEventAction>(sqlite3_column_int(statement, GetFileEvent_ColumnIndex_Action));
	return FileEvent(fullPath, contentBlobAddress, action);
}

}
}
}