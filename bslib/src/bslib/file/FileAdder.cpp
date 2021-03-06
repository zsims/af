#include "bslib/file/FileAdder.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"
#include "bslib/file/FilePathRepository.hpp"
#include "bslib/file/fs/operations.hpp"

#include <boost/filesystem.hpp>

#include <vector>
#include <map>

namespace af {
namespace bslib {
namespace file {

FileAdder::FileAdder(
	const Uuid& backupRunId,
	std::shared_ptr<blob::BlobStore> blobStore,
	blob::BlobInfoRepository& blobInfoRepository,
	FileEventStreamRepository& fileEventStreamRepository,
	FilePathRepository& filePathRepository)
	: _backupRunId(backupRunId)
	, _blobStore(blobStore)
	, _blobInfoRepository(blobInfoRepository)
	, _fileEventStreamRepository(fileEventStreamRepository)
	, _filePathRepository(filePathRepository)
{
}

boost::optional<blob::Address> FileAdder::SaveFileContents(const fs::NativePath& sourcePath)
{
	// TODO: ffs should really support files so they don't have to be read into memory. Or at least streaming...
	auto file = OpenFileRead(sourcePath);

	if (!file)
	{
		EmitEvent(RegularFileEvent(_backupRunId, sourcePath, boost::none, FileEventAction::FailedToRead));
		return boost::none;
	}

	const auto content = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	const auto blobAddress = blob::Address::CalculateFromContent(content);
	const auto existingBlob = _blobInfoRepository.FindBlob(blobAddress);
	if (!existingBlob)
	{
		_blobStore->CreateBlob(blobAddress, content);
		_blobInfoRepository.AddBlob(blob::BlobInfo(blobAddress, content.size()));
	}
	return blobAddress;
}

void FileAdder::Add(const UTF8String& sourcePath)
{
	// Get the full path
	auto absolutePath = fs::GetAbsolutePath(sourcePath);

	// Ensure forward slashes are back slashes (if applicable)
	absolutePath.MakePreferred();

	if (!fs::Exists(absolutePath))
	{
		throw PathNotFoundException(absolutePath.ToString());
	}

	if (fs::IsRegularFile(absolutePath))
	{
		const auto previousEvent = _fileEventStreamRepository.FindLastChangedEvent(absolutePath);
		VisitPath(absolutePath, previousEvent);
	}
	else if (fs::IsDirectory(absolutePath))
	{
		ScanDirectory(absolutePath.EnsureTrailingSlashCopy());
	}
	else
	{
		throw SourcePathNotSupportedException(absolutePath.ToString());
	}
}

void FileAdder::ScanDirectory(const fs::NativePath& sourcePath)
{
	auto lastChangeEvents = _fileEventStreamRepository.GetLastChangedEventsUnderPath(sourcePath);

	// The directory itself
	VisitPath(sourcePath, FindPreviousEvent(lastChangeEvents, sourcePath));

	// Scan for changes to files on disk
	boost::system::error_code ec;
	boost::filesystem::recursive_directory_iterator itr(sourcePath.ToExtendedString(), ec);
	if (ec)
	{
		EmitEvent(DirectoryEvent(_backupRunId, sourcePath, FileEventAction::FailedToRead));
		return;
	}

	for (; itr != boost::filesystem::end(itr); itr.increment(ec))
	{
		if (ec)
		{
			// TODO: log this, we don't have a path as the increment failed so an event can't be raised
			continue;
		}

		fs::NativePath path(WideToUTF8String(itr->path().wstring()));

		// Directories should always be processed with a slash
		if (fs::IsDirectory(path))
		{
			path.EnsureTrailingSlash();
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

void FileAdder::VisitPath(const fs::NativePath& sourcePath, const boost::optional<FileEvent>& previousEvent)
{
	if (!fs::Exists(sourcePath))
	{
		if (previousEvent && previousEvent->action != FileEventAction::ChangedRemoved)
		{
			EmitEvent(FileEvent(_backupRunId, sourcePath, previousEvent->type, previousEvent->contentBlobAddress, FileEventAction::ChangedRemoved));
		}
		return;
	}

	if (fs::IsRegularFile(sourcePath))
	{
		VisitFile(sourcePath, previousEvent);
	}
	else if (fs::IsDirectory(sourcePath))
	{
		VisitDirectory(sourcePath, previousEvent);
	}
	else
	{
		EmitEvent(FileEvent(_backupRunId, sourcePath, FileType::Unsupported, boost::none, FileEventAction::Unsupported));
	}
}

void FileAdder::VisitFile(const fs::NativePath& sourcePath, const boost::optional<FileEvent>& previousEvent)
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
					EmitEvent(RegularFileEvent(_backupRunId, sourcePath, previousEvent->contentBlobAddress, FileEventAction::Unchanged));
					return;
				}
				action = FileEventAction::ChangedModified;
				break;
			
			case FileEventAction::ChangedRemoved:
				break;
		}
	}

	EmitEvent(RegularFileEvent(_backupRunId, sourcePath, blobAddress, action));
}

void FileAdder::VisitDirectory(const fs::NativePath& sourcePath, const boost::optional<FileEvent>& previousEvent)
{
	if (!previousEvent)
	{
		EmitEvent(DirectoryEvent(_backupRunId, sourcePath.EnsureTrailingSlashCopy(), FileEventAction::ChangedAdded));
	}
}

boost::optional<FileEvent> FileAdder::FindPreviousEvent(
	const std::map<fs::NativePath, FileEvent>& fileEvents,
	const fs::NativePath& fullPath)
{
	const auto it = fileEvents.find(fullPath);
	if (it == fileEvents.end())
	{
		return boost::none;
	}
	return it->second;
}

void FileAdder::EmitEvent(const FileEvent& fileEvent)
{
	const auto pathId = _filePathRepository.AddPathTree(fileEvent.fullPath, fileEvent.type, _knownPaths);
	_emittedEvents.push_back(fileEvent);
	_fileEventStreamRepository.AddEvent(fileEvent, pathId);
	_eventManager.Publish(fileEvent);
}

}
}
}

