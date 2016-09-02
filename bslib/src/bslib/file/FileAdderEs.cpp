#include "bslib/file/FileAdderEs.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/DirectoryPath.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"

#include <boost/filesystem.hpp>

#include <vector>
#include <map>
#include <set>

namespace af {
namespace bslib {
namespace file {

FileAdderEs::FileAdderEs(
	blob::BlobStore& blobStore,
	blob::BlobInfoRepository& blobInfoRepository,
	FileEventStreamRepository& fileEventStreamRepository)
	: _blobStore(blobStore)
	, _blobInfoRepository(blobInfoRepository)
	, _fileEventStreamRepository(fileEventStreamRepository)
{
}

boost::optional<BlobAddress> FileAdderEs::SaveFileContents(const boost::filesystem::path& sourcePath)
{
	// TODO: ffs should really support files so they don't have to be read into memory. Or at least streaming...
	std::ifstream file(sourcePath.string(), std::ios::binary | std::ios::in);

	if (!file)
	{
		// TODO: log this as "uh oh"
		return boost::none;
	}

	const auto content = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	const auto blobAddress = BlobAddress::CalculateFromContent(content);
	const auto existingBlob = _blobInfoRepository.FindBlob(blobAddress);
	if (!existingBlob)
	{
		_blobStore.CreateBlob(blobAddress, content);
		_blobInfoRepository.AddBlob(blob::BlobInfo(blobAddress, content.size()));
	}
	return blobAddress;
}

void FileAdderEs::Add(const boost::filesystem::path& sourcePath)
{
	if (!boost::filesystem::exists(sourcePath))
	{
		throw PathNotFoundException(sourcePath.string());
	}

	// Clear from last run
	_lastEvents.clear();
	_newEvents.clear();

	if (boost::filesystem::is_regular_file(sourcePath))
	{
		const auto previousEvent = _fileEventStreamRepository.FindLastEvent(sourcePath);
		if (previousEvent)
		{
			_lastEvents.insert(std::make_pair(previousEvent->fullPath, previousEvent.value()));
		}
		VisitPath(sourcePath);
	}
	else if (boost::filesystem::is_directory(sourcePath))
	{
		ScanDirectory(DirectoryPath(sourcePath));
	}
	else
	{
		throw SourcePathNotSupportedException(sourcePath.string());
	}

	_fileEventStreamRepository.AddEvents(_newEvents);
}

void FileAdderEs::ScanDirectory(const boost::filesystem::path& sourcePath)
{
	_lastEvents = _fileEventStreamRepository.GetLastEventsStartingWithPath(sourcePath);

	// The directory itself
	VisitPath(sourcePath);


	// Scan for changes to files on disk
	std::set<boost::filesystem::path> processedPaths;
	boost::filesystem::recursive_directory_iterator itr(sourcePath);
	for (const auto& path : itr)
	{
		VisitPath(path);
		processedPaths.insert(path);
	}

	// Work out what happend to paths we once knew about but haven't seen again
	for (const auto& previousEvent : _lastEvents)
	{
		// already seen this event, skip
		if (processedPaths.count(previousEvent.first) != 0)
		{
			continue;
		}
		VisitPath(previousEvent.first);
	}
}

void FileAdderEs::VisitPath(const boost::filesystem::path& sourcePath)
{
	const auto& previousEvent = FindPreviousEvent(sourcePath);

	if (!boost::filesystem::exists(sourcePath))
	{
		if (previousEvent && previousEvent->action != FileEventAction::Removed)
		{
			PushEvent(FileEvent(sourcePath, previousEvent->contentBlobAddress, FileEventAction::Removed));
		}
		else
		{
			// Unchanged since last time
			return;
		}
	}

	if (boost::filesystem::is_regular_file(sourcePath))
	{
		VisitFile(sourcePath, previousEvent);
	}
	else if (boost::filesystem::is_directory(sourcePath))
	{
		VisitDirectory(sourcePath);
	}
}

void FileAdderEs::VisitFile(const boost::filesystem::path& sourcePath, const boost::optional<FileEvent>& previousEvent)
{
	const auto blobAddress = SaveFileContents(sourcePath);
	if (!blobAddress)
	{
		// Expected some contents as this is a file
		// TODO: Record this
		return;
	}

	auto action = FileEventAction::Added;
	if (previousEvent && blobAddress != previousEvent->contentBlobAddress)
	{
		action = FileEventAction::Modified;
	}

	PushEvent(FileEvent(sourcePath, blobAddress, action));
}

void FileAdderEs::VisitDirectory(const boost::filesystem::path& sourcePath)
{
	PushEvent(FileEvent(DirectoryPath(sourcePath), boost::none, FileEventAction::Added));
}

boost::optional<FileEvent> FileAdderEs::FindPreviousEvent(const boost::filesystem::path& fullPath) const
{
	const auto it = _lastEvents.find(fullPath);
	if (it == _lastEvents.end())
	{
		return boost::none;
	}
	return it->second;
}

void FileAdderEs::PushEvent(const FileEvent& fileEvent)
{
	_newEvents.push_back(fileEvent);
}

}
}
}

