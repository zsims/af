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

/**
 * Finds files in a backup
 */
class FileFinder
{
public:
	explicit FileFinder(FileEventStreamRepository& fileEventStreamRepository);

	/**
	 * Gets the last recorded event for the file at the given path
	 */
	FileEvent GetLastEventByPath(const fs::NativePath& fullPath) const;
	boost::optional<FileEvent> FindLastChangedEventByPath(const fs::NativePath& fullPath) const;
	std::map<fs::NativePath, FileEvent> GetLastChangedEventsStartingWithPath(const fs::NativePath& fullPath) const;
	std::vector<FileEvent> GetAllEvents() const;
private:
	const FileEventStreamRepository& _fileEventStreamRepository;
};

}
}
}

