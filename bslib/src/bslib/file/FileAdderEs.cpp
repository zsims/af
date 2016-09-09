#include "bslib/file/FileAdderEs.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/DirectoryPath.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"

#include <boost/filesystem.hpp>

#include <vector>
#include <map>

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
		EmitEvent(FileEvent(sourcePath, FileType::RegularFile, boost::none, FileEventAction::FailedToRead));
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

	if (boost::filesystem::is_regular_file(sourcePath))
	{
		const auto previousEvent = _fileEventStreamRepository.FindLastChangedEvent(sourcePath);
		VisitPath(sourcePath, previousEvent);
	}
	else if (boost::filesystem::is_directory(sourcePath))
	{
		ScanDirectory(DirectoryPath(sourcePath));
	}
	else
	{
		throw SourcePathNotSupportedException(sourcePath.string());
	}

}

void FileAdderEs::ScanDirectory(const boost::filesystem::path& sourcePath)
{
	auto lastChangeEvents = _fileEventStreamRepository.GetLastChangedEventsStartingWithPath(sourcePath);

	// The directory itself
	VisitPath(sourcePath, FindPreviousEvent(lastChangeEvents, sourcePath));

	// Scan for changes to files on disk
	boost::filesystem::recursive_directory_iterator itr(sourcePath);
	for (const auto& entry : itr)
	{
		auto path = entry.path();

		// Directories should always be processed with a slash
		if (boost::filesystem::is_directory(path))
		{
			path = DirectoryPath(path);
		}

		VisitPath(path, FindPreviousEvent(lastChangeEvents, path));
		lastChangeEvents.erase(path);
	}

	// Work out what happend to paths we once knew about but haven't seen again
	for (const auto& previousEvent : lastChangeEvents)
	{
		VisitPath(previousEvent.first, previousEvent.second);
	}
}

void FileAdderEs::VisitPath(const boost::filesystem::path& sourcePath, const boost::optional<FileEvent>& previousEvent)
{
	if (!boost::filesystem::exists(sourcePath))
	{
		if (previousEvent && previousEvent->action != FileEventAction::ChangedRemoved)
		{
			EmitEvent(FileEvent(sourcePath, previousEvent->type, previousEvent->contentBlobAddress, FileEventAction::ChangedRemoved));
		}
		return;
	}

	if (boost::filesystem::is_regular_file(sourcePath))
	{
		VisitFile(sourcePath, previousEvent);
	}
	else if (boost::filesystem::is_directory(sourcePath))
	{
		VisitDirectory(sourcePath, previousEvent);
	}
	else
	{
		EmitEvent(FileEvent(sourcePath, FileType::Unsupported, boost::none, FileEventAction::Unsupported));
	}
}

void FileAdderEs::VisitFile(const boost::filesystem::path& sourcePath, const boost::optional<FileEvent>& previousEvent)
{
	const auto blobAddress = SaveFileContents(sourcePath);
	if (!blobAddress)
	{
		return;
	}

	// Assume added
	auto action = FileEventAction::ChangedAdded;
	if (previousEvent)
	{
		switch (previousEvent->action)
		{
			case FileEventAction::ChangedAdded:
			case FileEventAction::ChangedModified:
				if (previousEvent->contentBlobAddress == blobAddress)
				{
					EmitEvent(FileEvent(sourcePath, FileType::RegularFile, previousEvent->contentBlobAddress, FileEventAction::Unchanged));
					return;
				}
				action = FileEventAction::ChangedModified;
				break;
			
			case FileEventAction::ChangedRemoved:
				break;
		}
	}

	EmitEvent(FileEvent(sourcePath, FileType::RegularFile, blobAddress, action));
}

void FileAdderEs::VisitDirectory(const boost::filesystem::path& sourcePath, const boost::optional<FileEvent>& previousEvent)
{
	if (!previousEvent)
	{
		EmitEvent(FileEvent(DirectoryPath(sourcePath), FileType::Directory, boost::none, FileEventAction::ChangedAdded));
	}
}

boost::optional<FileEvent> FileAdderEs::FindPreviousEvent(
	const std::map<boost::filesystem::path, FileEvent>& fileEvents,
	const boost::filesystem::path& fullPath)
{
	const auto it = fileEvents.find(fullPath);
	if (it == fileEvents.end())
	{
		return boost::none;
	}
	return it->second;
}

void FileAdderEs::EmitEvent(const FileEvent& fileEvent)
{
	_emittedEvents.push_back(fileEvent);
	_fileEventStreamRepository.AddEvent(fileEvent);
	_eventManager.Publish(fileEvent);
}

}
}
}

