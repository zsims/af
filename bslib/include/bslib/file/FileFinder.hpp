#pragma once

#include "bslib/Address.hpp"
#include "bslib/file/FileEvent.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <map>
#include <vector>

namespace af {
namespace bslib {
namespace file {

class FileRefRepository;
class FileObjectRepository;
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
	FileEvent GetLastEventByPath(const boost::filesystem::path& fullPath) const;
	boost::optional<FileEvent> FindLastChangedEventByPath(const boost::filesystem::path& fullPath) const;
	std::map<boost::filesystem::path, FileEvent> GetLastChangedEventsStartingWithPath(const boost::filesystem::path& fullPath) const;
	std::vector<FileEvent> GetAllEvents() const;
private:
	const FileEventStreamRepository& _fileEventStreamRepository;
};

}
}
}

