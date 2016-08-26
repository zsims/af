#include "bslib/file/FileRestorer.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileRefRepository.hpp"
#include "bslib/file/FileObjectInfoRepository.hpp"

#include <boost/filesystem.hpp>

#include <vector>

namespace af {
namespace bslib {
namespace file {

FileRestorer::FileRestorer(
	blob::BlobStore& blobStore,
	blob::BlobInfoRepository& blobInfoRepository,
	FileObjectInfoRepository& fileObjectInfoRepository,
	FileRefRepository& fileRefRepository)
	: _blobStore(blobStore)
	, _blobInfoRepository(blobInfoRepository)
	, _fileObjectInfoRepository(fileObjectInfoRepository)
	, _fileRefRepository(fileRefRepository)
{
}

void FileRestorer::RestoreSingle(const ObjectAddress& objectAddress, const boost::filesystem::path& targetPath)
{
	Restore(objectAddress, targetPath, false);
}

void FileRestorer::RestoreTree(const ObjectAddress& objectAddress, const boost::filesystem::path& targetPath)
{
	Restore(objectAddress, targetPath, true);
}

void FileRestorer::Restore(const ObjectAddress& objectAddress, const boost::filesystem::path& targetPath, bool recursive)
{
	if (boost::filesystem::exists(targetPath) && !boost::filesystem::is_directory(targetPath))
	{
		throw TargetPathNotSupportedException(targetPath.string());
	}

	const auto object = _fileObjectInfoRepository.GetObject(objectAddress);
	auto resolvedTargetPath = targetPath;
	if (boost::filesystem::exists(targetPath))
	{
		resolvedTargetPath = targetPath / boost::filesystem::path(object.fullPath).filename();
	}
	else
	{
		// Create intermediate directories if we're restoring to a path that doesn't exist
		boost::system::error_code ec;
		boost::filesystem::create_directories(resolvedTargetPath.parent_path(), ec);
		if (ec != boost::system::errc::success)
		{
			_skippedPaths.push_back(resolvedTargetPath);
			return;
		}
	}

	RestoreFileObject(object, resolvedTargetPath, recursive);
}

void FileRestorer::RestoreFileObject(const FileObjectInfo& info, const boost::filesystem::path& targetPath, bool followDirectories)
{
	// File
	if (info.contentBlobAddress)
	{
		if (!RestoreBlobToFile(info.contentBlobAddress.value(), targetPath))
		{
			_skippedPaths.push_back(targetPath);
			return;
		}
		_restoredPaths.push_back(targetPath);
	}
	else
	{
		// Directory
		boost::system::error_code ec;
		boost::filesystem::create_directory(targetPath, ec);
		if (ec != boost::system::errc::success)
		{
			_skippedPaths.push_back(targetPath);
			return;
		}
		
		// Yes, it hasn't actually been restored full yet, but I want ma tail call optimization
		_restoredPaths.push_back(targetPath);

		const auto& children = _fileObjectInfoRepository.GetAllObjectsByParentAddress(info.address);
		for (const auto& child : children)
		{
			const auto& name = boost::filesystem::path(child->fullPath).filename();
			// *Hope* for some sweet tail call optimization :-D
			RestoreFileObject(*child, targetPath / name, followDirectories);
		}
	}
}

bool FileRestorer::RestoreBlobToFile(const BlobAddress& blobAddress, const boost::filesystem::path& targetPath) const
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

}
}
}

