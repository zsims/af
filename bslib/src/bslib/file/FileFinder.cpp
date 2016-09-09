#include "bslib/file/FileFinder.hpp"

#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"

namespace af {
namespace bslib {
namespace file {

FileFinder::FileFinder(
	FileEventStreamRepository& fileEventStreamRepository)
	: _fileEventStreamRepository(fileEventStreamRepository)
{
}

boost::optional<FileEvent> FileFinder::FindLastChangedEventByPath(const boost::filesystem::path& fullPath) const
{
	return _fileEventStreamRepository.FindLastChangedEvent(fullPath);
}

std::map<boost::filesystem::path, FileEvent> FileFinder::GetLastChangedEventsStartingWithPath(const boost::filesystem::path& fullPath) const
{
	return _fileEventStreamRepository.GetLastChangedEventsStartingWithPath(fullPath);
}

FileEvent FileFinder::GetLastEventByPath(const boost::filesystem::path& fullPath) const
{
	const auto ev = _fileEventStreamRepository.FindLastChangedEvent(fullPath);
	if (!ev)
	{
		throw EventNotFoundException("Event with path " + fullPath.string() + " not found");
	}
	return ev.value();
}

std::vector<FileEvent> FileFinder::GetAllEvents() const
{
	return _fileEventStreamRepository.GetAllEvents();
}

}
}
}

