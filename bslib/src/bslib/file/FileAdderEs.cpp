#include "bslib/file/FileAdderEs.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/path_util.hpp"
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

boost::optional<blob::Address> FileAdderEs::SaveFileContents(const boost::filesystem::path& sourcePath)
{
	// TODO: ffs should really support files so they don't have to be read into memory. Or at least streaming...
	std::ifstream file(sourcePath.string(), std::ios::binary | std::ios::in);

	if (!file)
	{
		EmitEvent(RegularFileEvent(sourcePath, boost::none, FileEventAction::FailedToRead));
		return boost::none;
	}

	const auto content = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	const auto blobAddress = blob::Address::CalculateFromContent(content);
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
	// Get the full path resolving symlinks and . + .. references
	// This also checks the path exists
	boost::system::error_code ec;
	auto canonicalPath = boost::filesystem::canonical(sourcePath, ec);
	if (ec != boost::system::errc::success)
	{
		throw PathNotFoundException(canonicalPath.string());
	}

	// Ensure slashes are using the system preferred / or \, this is important as canonical() doesn't do this
	// (in fact, it changes the root \ to / on Windows :/)
	canonicalPath.make_preferred();

	if (boost::filesystem::is_regular_file(canonicalPath))
	{
		const auto previousEvent = _fileEventStreamRepository.FindLastChangedEvent(canonicalPath);
		VisitPath(canonicalPath, previousEvent);
	}
	else if (boost::filesystem::is_directory(canonicalPath))
	{
		ScanDirectory(EnsureTrailingSlashCopy(canonicalPath));
	}
	else
	{
		throw SourcePathNotSupportedException(canonicalPath.string());
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
			EnsureTrailingSlash(path);
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
					EmitEvent(RegularFileEvent(sourcePath, previousEvent->contentBlobAddress, FileEventAction::Unchanged));
					return;
				}
				action = FileEventAction::ChangedModified;
				break;
			
			case FileEventAction::ChangedRemoved:
				break;
		}
	}

	EmitEvent(RegularFileEvent(sourcePath, blobAddress, action));
}

void FileAdderEs::VisitDirectory(const boost::filesystem::path& sourcePath, const boost::optional<FileEvent>& previousEvent)
{
	if (!previousEvent)
	{
		EmitEvent(DirectoryEvent(EnsureTrailingSlashCopy(sourcePath), FileEventAction::ChangedAdded));
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

