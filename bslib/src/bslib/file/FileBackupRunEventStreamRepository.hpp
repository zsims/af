#pragma once

#include "bslib/file/FileBackupRunEvent.hpp"
#include "bslib/file/FileBackupRunSearchCriteria.hpp"
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
	 * Search for a list of events. All events matching the *run* in the criteria will be returned.
	 * \remarks "By run" ensures the results will not span runs across multiple pages, making it a PITA to work with results
	 * Note that only *started* runs will be considered.
	 */
	std::vector<FileBackupRunEvent> SearchByRun(const FileBackupRunSearchCriteria& criteria, unsigned skipRuns, unsigned uniqueRunLimit) const;

	/**
	 * Gets file backup events by the given run id.
	 * Events are ordered from oldest to newest
	 */
	std::vector<FileBackupRunEvent> GetByRunId(const Uuid& runId) const;

	/**
	 * Gets a count of how many unique backup runs there have been
	 */
	unsigned GetBackupCount() const;
private:
	FileBackupRunEvent MapRowToEvent(const sqlitepp::ScopedStatement& statement) const;

	const sqlitepp::ScopedSqlite3Object& _db;
	sqlitepp::ScopedStatement _insertEventStatement;
	sqlitepp::ScopedStatement _getAllEventsStatement;
};

}
}
}