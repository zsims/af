#pragma once

#include "bslib/file/FileEvent.hpp"
#include "bslib/file/FileEventSearchCriteria.hpp"
#include "bslib/file/fs/path.hpp"
#include "bslib/sqlitepp/handles.hpp"

#include <cstdint>
#include <map>
#include <set>
#include <vector>

#include <boost/optional.hpp>

namespace af {
namespace bslib {
namespace file {

class FileEventStreamRepository
{
public:
	struct RunStats
	{
		unsigned matchingEvents;
		uint64_t matchingSizeBytes;
	};

	explicit FileEventStreamRepository(const sqlitepp::ScopedSqlite3Object& connection);

	/**
	 * Gets all events in the order they were recorded
	 */
	std::vector<FileEvent> GetAllEvents() const;
	std::vector<FileEvent> GetEventsByRunId(const Uuid& runId) const;
	std::map<fs::NativePath, FileEvent> GetLastChangedEventsStartingWithPath(const fs::NativePath& fullPath) const;
	boost::optional<FileEvent> FindLastChangedEvent(const fs::NativePath& fullPath) const;

	void AddEvent(const FileEvent& fileEvent);

	template<typename Container>
	void AddEvents(const Container& events)
	{
		for (const auto& e : events)
		{
			AddEvent(e);
		}
	}

	/**
	 * Gets statics by the given run ids with the given actions
	 * \param a vector of run ids to return
	 * \param a set of actions to count
	 * \return A map of run id -> stats
	 */
	std::map<Uuid, RunStats> GetStatisticsByRunId(const std::vector<Uuid>& runIds, const std::set<FileEventAction>& actions) const;

	std::vector<FileEvent> Search(const FileEventSearchCriteria& criteria, unsigned skip, unsigned limit) const;
	unsigned CountMatching(const FileEventSearchCriteria& criteria) const;
private:
	FileEvent MapRowToEvent(const sqlitepp::ScopedStatement& statement) const;

	const sqlitepp::ScopedSqlite3Object& _db;
	sqlitepp::ScopedStatement _insertEventStatement;
	sqlitepp::ScopedStatement _getAllEventsStatement;
	sqlitepp::ScopedStatement _getLastChangedEventByPathStatement;
	sqlitepp::ScopedStatement _getLastChangedEventsUnderPathStatement;
};

}
}
}