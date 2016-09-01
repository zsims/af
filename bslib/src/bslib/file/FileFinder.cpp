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

boost::optional<FileEvent> FileFinder::FindLastEventByPath(const boost::filesystem::path& fullPath) const
{
	return _fileEventStreamRepository.FindLastEvent(fullPath);
}

std::map<boost::filesystem::path, FileEvent> FileFinder::GetLastEventsStartingWithPath(const boost::filesystem::path& fullPath) const
{
	return _fileEventStreamRepository.GetLastEventsStartingWithPath(fullPath);
}

FileEvent FileFinder::GetLastEventByPath(const boost::filesystem::path& fullPath) const
{
	const auto ev = _fileEventStreamRepository.FindLastEvent(fullPath);
	if (!ev)
	{
		throw EventNotFoundException("Event with path " + fullPath.string() + " not found");
	}
	return ev.value();
}

}
}
}

