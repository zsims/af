#include "bslib/file/FileRestorer.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/fs/operations.hpp"

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <vector>

namespace af {
namespace bslib {
namespace file {

FileRestorer::FileRestorer(
	std::shared_ptr<blob::BlobStore> blobStore,
	blob::BlobInfoRepository& blobInfoRepository)
	: _blobStore(blobStore)
	, _blobInfoRepository(blobInfoRepository)
{
}

void FileRestorer::Restore(const FileEvent& fileEvent, const UTF8String& targetPath)
{
	auto absolutePath = fs::GetAbsolutePath(targetPath);
	absolutePath.MakePreferred();
	RestoreFileEvent(fileEvent, absolutePath);
}

void FileRestorer::Restore(const std::map<fs::NativePath, FileEvent>& fileEvents, const UTF8String& targetPath)
{
	std::vector<FileEvent> events;
	boost::copy(fileEvents | boost::adaptors::map_values, std::back_inserter(events));
	Restore(events, targetPath);
}

void FileRestorer::Restore(const std::vector<FileEvent>& fileEvents, const UTF8String& targetPath)
{
	auto absolutePath = fs::GetAbsolutePath(targetPath);
	absolutePath.MakePreferred();

	if (!fs::IsDirectory(absolutePath))
	{
		throw TargetPathNotSupportedException(absolutePath.ToString());
	}

	for (const auto& fileEvent : fileEvents)
	{
		auto fileEventTargetPath = absolutePath.AppendFullCopy(fileEvent.fullPath);
		RestoreFileEvent(fileEvent, fileEventTargetPath);
	}
}

void FileRestorer::RestoreFileEvent(const FileEvent& fileEvent, const fs::NativePath& targetPath)
{
	if (!IsFileEventActionSupported(fileEvent.action))
	{
		EmitEvent(FileRestoreEvent(fileEvent, targetPath, FileRestoreEventAction::UnsupportedFileEvent));
		return;
	}

	if (fs::Exists(targetPath))
	{
		EmitEvent(FileRestoreEvent(fileEvent, targetPath, FileRestoreEventAction::Skipped));
		return;
	}

	// File
	if(fileEvent.type == FileType::RegularFile && fileEvent.contentBlobAddress)
	{
		boost::system::error_code ec;
		fs::CreateDirectories(targetPath.ParentPathCopy(), ec);
		if (ec)
		{
			EmitEvent(FileRestoreEvent(fileEvent, targetPath, FileRestoreEventAction::FailedToCreateDirectory));
			return;
		}

		if (!RestoreBlobToFile(fileEvent.contentBlobAddress.value(), targetPath))
		{
			EmitEvent(FileRestoreEvent(fileEvent, targetPath, FileRestoreEventAction::FailedToWriteFile));
			return;
		}
		EmitEvent(FileRestoreEvent(fileEvent, targetPath, FileRestoreEventAction::Restored));
	}
	else if(fileEvent.type == FileType::Directory)
	{
		// Directory
		boost::system::error_code ec;
		fs::CreateDirectories(targetPath, ec);
		if (ec)
		{
			EmitEvent(FileRestoreEvent(fileEvent, targetPath, FileRestoreEventAction::FailedToCreateDirectory));
			return;
		}
		EmitEvent(FileRestoreEvent(fileEvent, targetPath, FileRestoreEventAction::Restored));
	}
	else
	{
		EmitEvent(FileRestoreEvent(fileEvent, targetPath, FileRestoreEventAction::UnsupportedFileEvent));
	}
}

bool FileRestorer::RestoreBlobToFile(const blob::Address& blobAddress, const fs::NativePath& targetPath) const
{
	const auto content = _blobStore->GetBlob(blobAddress);
	auto file = fs::OpenFileWrite(targetPath);
	if (!file)
	{
		return false;
	}
	file.write(reinterpret_cast<const char*>(&content[0]), content.size());
	return true;
}

void FileRestorer::EmitEvent(const FileRestoreEvent& fileRestoreEvent)
{
	_emittedEvents.push_back(fileRestoreEvent);
	_eventManager.Publish(fileRestoreEvent);
}

bool FileRestorer::IsFileEventActionSupported(FileEventAction action)
{
	switch (action)
	{
		case FileEventAction::ChangedAdded:
		case FileEventAction::ChangedModified:
			return true;
	}
	return false;
}

}
}
}

