#include "bslib/file/FileRestorerEs.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/fs/operations.hpp"
#include "bslib/file/path_util.hpp"

#include <vector>

namespace af {
namespace bslib {
namespace file {

FileRestorerEs::FileRestorerEs(
	blob::BlobStore& blobStore,
	blob::BlobInfoRepository& blobInfoRepository)
	: _blobStore(blobStore)
	, _blobInfoRepository(blobInfoRepository)
{
}

void FileRestorerEs::Restore(const FileEvent& fileEvent, const fs::NativePath& targetPath)
{
	RestoreFileEvent(fileEvent, targetPath);
}

void FileRestorerEs::Restore(const std::vector<FileEvent>& fileEvents, const fs::NativePath& targetPath)
{
	if (!fs::IsDirectory(targetPath))
	{
		throw TargetPathNotSupportedException(targetPath.ToExtendedString());
	}

	for (const auto& fileEvent : fileEvents)
	{
		auto fileEventTargetPath = targetPath;
		fileEventTargetPath.AppendFull(fileEvent.fullPath);
		RestoreFileEvent(fileEvent, fileEventTargetPath);
	}
}

void FileRestorerEs::RestoreFileEvent(const FileEvent& fileEvent, const fs::NativePath& targetPath)
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

bool FileRestorerEs::RestoreBlobToFile(const blob::Address& blobAddress, const fs::NativePath& targetPath) const
{
	const auto content = _blobStore.GetBlob(blobAddress);
	auto file = fs::OpenFileWrite(targetPath);
	if (!file)
	{
		return false;
	}
	file.write(reinterpret_cast<const char*>(&content[0]), content.size());
	return true;
}

void FileRestorerEs::EmitEvent(const FileRestoreEvent& fileRestoreEvent)
{
	_emittedEvents.push_back(fileRestoreEvent);
	_eventManager.Publish(fileRestoreEvent);
}

bool FileRestorerEs::IsFileEventActionSupported(FileEventAction action)
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

