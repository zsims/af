#pragma once

#include "bslib/file/FileEvent.hpp"
#include "bslib/file/fs/path.hpp"
#include "bslib/sqlitepp/handles.hpp"

#include <cstdint>
#include <map>
#include <vector>

#include <boost/optional.hpp>

namespace af {
namespace bslib {
namespace file {

class FileEventStreamRepository
{
public:
	explicit FileEventStreamRepository(const sqlitepp::ScopedSqlite3Object& connection);

	/**
	 * Gets all events in the order they were recorded
	 */
	std::vector<FileEvent> GetAllEvents() const;
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