#pragma once

#include "bslib/file/FileBackupRunEvent.hpp"
#include "bslib/sqlitepp/handles.hpp"

#include <vector>

namespace af {
namespace bslib {
namespace file {

class FileBackupRunEventStreamRepository
{
public:
	explicit FileBackupRunEventStreamRepository(const sqlitepp::ScopedSqlite3Object& connection);

	/**
	 * Gets all events in the order they were recorded
	 */
	std::vector<FileBackupRunEvent> GetAllEvents() const;

	void AddEvent(const FileBackupRunEvent& backupEvent);

	template<typename Container>
	void AddEvents(const Container& events)
	{
		for (const auto& e : events)
		{
			AddEvent(e);
		}
	}

	/**
	 * Gets a list of events
	 * \param skipRuns The number of (unique) runs to skip
	 * \param uniqueRunLimit The limit of unique runs to return
	 */
	std::vector<FileBackupRunEvent> GetPaged(unsigned skipRuns, unsigned uniqueRunLimit) const;
private:
	FileBackupRunEvent MapRowToEvent(const sqlitepp::ScopedStatement& statement) const;

	const sqlitepp::ScopedSqlite3Object& _db;
	sqlitepp::ScopedStatement _insertEventStatement;
	sqlitepp::ScopedStatement _getAllEventsStatement;
};

}
}
}