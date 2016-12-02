#include "bslib/file/FileFinder.hpp"

#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"
#include "bslib/file/FilePathRepository.hpp"

namespace af {
namespace bslib {
namespace file {

FileFinder::FileFinder(
	FileEventStreamRepository& fileEventStreamRepository,
	FilePathRepository& filePathRepository)
	: _fileEventStreamRepository(fileEventStreamRepository)
	, _filePathRepository(filePathRepository)
{
}

boost::optional<FileEvent> FileFinder::FindLastChangedEventByPath(const fs::NativePath& fullPath) const
{
	return _fileEventStreamRepository.FindLastChangedEvent(fullPath);
}

std::map<fs::NativePath, FileEvent> FileFinder::GetLastChangedEventsUnderPath(const fs::NativePath& fullPath) const
{
	return _fileEventStreamRepository.GetLastChangedEventsUnderPath(fullPath);
}

FileEvent FileFinder::GetLastEventByPath(const fs::NativePath& fullPath) const
{
	const auto ev = _fileEventStreamRepository.FindLastChangedEvent(fullPath);
	if (!ev)
	{
		throw EventNotFoundException("Event with path " + fullPath.ToString() + " not found");
	}
	return ev.value();
}

std::vector<FileEvent> FileFinder::GetAllEvents() const
{
	return _fileEventStreamRepository.GetAllEvents();
}

FileFinder::ResultsPage FileFinder::SearchEvents(const FileEventSearchCriteria& criteria, unsigned skip, unsigned pageSize) const
{
	ResultsPage results;
	results.events = _fileEventStreamRepository.Search(criteria, skip, pageSize);
	results.totalEvents = _fileEventStreamRepository.CountMatching(criteria);
	const auto eventsSize = static_cast<unsigned>(results.events.size());
	results.nextPageSkip = skip + std::min(eventsSize, pageSize);
	return results;
}

bool FileFinder::IsKnownPath(const fs::NativePath& fullPath) const
{
	if (_filePathRepository.FindPath(fullPath))
	{
		return true;
	}
	return false;
}

}
}
}

