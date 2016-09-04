#include "bslib/file/FileFinder.hpp"

#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"
#include "bslib/file/FileRefRepository.hpp"
#include "bslib/file/FileObjectRepository.hpp"

namespace af {
namespace bslib {
namespace file {

FileFinder::FileFinder(
	FileObjectRepository& fileObjectRepository,
	FileRefRepository& fileRefRepository,
	FileEventStreamRepository& fileEventStreamRepository)
	: _fileObjectRepository(fileObjectRepository)
	, _fileRefRepository(fileRefRepository)
	, _fileEventStreamRepository(fileEventStreamRepository)
{
}

boost::optional<FileRef> FileFinder::FindReference(const boost::filesystem::path& sourcePath) const
{
	return _fileRefRepository.FindReference(sourcePath.string());
}

boost::optional<FileObject> FileFinder::FindObjectById(foid id) const
{
	return _fileObjectRepository.FindObject(id);
}

FileObject FileFinder::GetObjectById(foid id) const
{
	return _fileObjectRepository.GetObject(id);
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

