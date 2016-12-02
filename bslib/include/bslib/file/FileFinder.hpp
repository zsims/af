#pragma once

#include "bslib/file/FileEvent.hpp"
#include "bslib/file/fs/path.hpp"

#include <boost/optional.hpp>

#include <map>
#include <vector>

namespace af {
namespace bslib {
namespace file {

class FileEventStreamRepository;
class FilePathRepository;
struct FileEventSearchCriteria;

/**
 * Finds files in a backup
 */
class FileFinder
{
public:
	FileFinder(FileEventStreamRepository& fileEventStreamRepository, FilePathRepository& filePathRepository);

	struct ResultsPage
	{
		ResultsPage()
			: totalEvents(0)
			, nextPageSkip(0)
		{
		}
		unsigned totalEvents;
		unsigned nextPageSkip;
		std::vector<FileEvent> events;
	};

	/**
	 * Gets the last recorded event for the file at the given path
	 */
	FileEvent GetLastEventByPath(const fs::NativePath& fullPath) const;
	boost::optional<FileEvent> FindLastChangedEventByPath(const fs::NativePath& fullPath) const;
	std::map<fs::NativePath, FileEvent> GetLastChangedEventsUnderPath(const fs::NativePath& fullPath) const;
	std::vector<FileEvent> GetAllEvents() const;
	ResultsPage SearchEvents(const FileEventSearchCriteria& criteria, unsigned skip, unsigned pageSize) const;

	/**
	 * Returns true if the given path is known, such that it or one of its children have been visited in a backup
	 */
	bool IsKnownPath(const fs::NativePath& fullPath) const;
private:
	const FileEventStreamRepository& _fileEventStreamRepository;
	const FilePathRepository& _filePathRepository;
};

}
}
}

