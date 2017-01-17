#pragma once

#include "bslib/file/FileEvent.hpp"
#include "bslib/file/FileEventSearchCriteria.hpp"
#include "bslib/file/FilePathSearchCriteria.hpp"
#include "bslib/file/fs/path.hpp"
#include "bslib/sqlitepp/handles.hpp"

#include <cstdint>
#include <map>
#include <set>
#include <vector>
#include <unordered_map>
#include <unordered_set>

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

	struct PathFirstSearchMatch
	{
		fs::NativePath fullPath;
		FileType pathType;
		int64_t pathId;
		boost::optional<FileEvent> latestEvent;
	};

	explicit FileEventStreamRepository(const sqlitepp::ScopedSqlite3Object& connection);

	/**
	 * Gets all events in the order they were recorded
	 */
	std::vector<FileEvent> GetAllEvents() const;
	std::map<fs::NativePath, FileEvent> GetLastChangedEventsUnderPath(const fs::NativePath& fullPath) const;
	boost::optional<FileEvent> FindLastChangedEvent(const fs::NativePath& fullPath) const;

	void AddEvent(const FileEvent& fileEvent, int64_t pathId);

	/**
	 * Gets statics by the given run ids with the given actions
	 * \param a vector of run ids to return
	 * \param a set of actions to count
	 * \return A map of run id -> stats
	 */
	std::map<Uuid, RunStats> GetStatisticsByRunId(const std::vector<Uuid>& runIds, const std::set<FileEventAction>& actions) const;

	/**
	 * Searches for events matching the given event *and* path criteria
	 */
	std::vector<FileEvent> Search(const FilePathSearchCriteria& pathCriteria, const FileEventSearchCriteria& eventCriteria, unsigned skip, unsigned limit) const;
	std::vector<FileEvent> Search(const FileEventSearchCriteria& eventCriteria, unsigned skip, unsigned limit) const;
	unsigned CountMatching(const FilePathSearchCriteria& pathCriteria, const FileEventSearchCriteria& eventCriteria) const;
	unsigned CountMatching(const FileEventSearchCriteria& eventCriteria) const;

	/**
	 * Searches for all matching paths and associated *latest* event
	 */
	std::vector<PathFirstSearchMatch> SearchPathFirst(
		const FilePathSearchCriteria& pathCriteria,
		const FileEventSearchCriteria& eventCriteria,
		unsigned skip,
		unsigned limit) const;
	unsigned CountMatching(const FilePathSearchCriteria& criteria) const;

	/**
	* Returns a set of path ids from the given set of path ids with the associated count of *sub* matching events
	* Note that the given paths should not overlap, otherwise results are undefined
	*/
	std::unordered_map<int64_t, unsigned> CountNestedMatches(const FileEventSearchCriteria& eventCriteria, const std::unordered_set<int64_t>& pathIds);
private:
	FileEvent MapRowToEvent(const sqlitepp::ScopedStatement& statement) const;

	const sqlitepp::ScopedSqlite3Object& _db;
	sqlitepp::ScopedStatement _getAllEventsStatement;
	sqlitepp::ScopedStatement _getLastChangedEventByPathStatement;
	sqlitepp::ScopedStatement _getLastChangedEventsUnderPathStatement;
};

}
}
}