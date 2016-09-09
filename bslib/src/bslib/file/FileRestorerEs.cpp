#include "bslib/file/FileRestorerEs.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/path_util.hpp"

#include <boost/filesystem.hpp>

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

void FileRestorerEs::Restore(const FileEvent& fileEvent, const boost::filesystem::path& targetPath)
{
	RestoreFileEvent(fileEvent, targetPath);
}

void FileRestorerEs::Restore(const std::vector<FileEvent>& fileEvents, const boost::filesystem::path& targetPath)
{
	if (!boost::filesystem::is_directory(targetPath))
	{
		throw TargetPathNotSupportedException(targetPath.string());
	}

	for (const auto& fileEvent : fileEvents)
	{
		const auto fileEventTargetPath = CombineFullPaths(targetPath, fileEvent.fullPath);
		RestoreFileEvent(fileEvent, fileEventTargetPath);
	}
}

void FileRestorerEs::RestoreFileEvent(const FileEvent& fileEvent, const boost::filesystem::path& targetPath)
{
	if (!IsFileEventActionSupported(fileEvent.action))
	{
		EmitEvent(FileRestoreEvent(fileEvent, targetPath, FileRestoreEventAction::UnsupportedFileEvent));
		return;
	}

	if (boost::filesystem::exists(targetPath))
	{
		EmitEvent(FileRestoreEvent(fileEvent, targetPath, FileRestoreEventAction::Skipped));
		return;
	}

	// File
	if(fileEvent.type == FileType::RegularFile && fileEvent.contentBlobAddress)
	{
		boost::system::error_code ec;
		boost::filesystem::create_directories(targetPath.parent_path(), ec);
		if (ec != boost::system::errc::success)
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
		boost::filesystem::create_directories(targetPath, ec);
		if (ec != boost::system::errc::success)
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

bool FileRestorerEs::RestoreBlobToFile(const blob::Address& blobAddress, const boost::filesystem::path& targetPath) const
{
	const auto content = _blobStore.GetBlob(blobAddress);
	std::ofstream file(targetPath.string(), std::ios::binary | std::ios::out);
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

