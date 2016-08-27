#pragma once

#include "bslib/Address.hpp"

#include <boost/filesystem/path.hpp>

#include <vector>

namespace af {
namespace bslib {
namespace blob {
class BlobInfoRepository;
class BlobStore;
}
namespace file {

struct FileObject;
class FileRefRepository;
class FileObjectRepository;

/**
 * Restores files and directories from the backup
 */
class FileRestorer
{
public:
	FileRestorer(
		blob::BlobStore& blobStore,
		blob::BlobInfoRepository& blobInfoRepository,
		FileObjectRepository& fileObjectRepository,
		FileRefRepository& fileRefRepository);

	/**
	 * Restores the single object represented by the given address to the given path
	 * \param objectAddress Address of the file object to restore
	 * \param targetPath The path to restore to, if existing files will be created as a subpath otherwise the full path will be restored
	 */
	void RestoreSingle(const ObjectAddress& objectAddress, const boost::filesystem::path& targetPath);

	/**
	 * Restores the given object, and all child objects (e.g. sub folders/files) to the target path
	 * \param objectAddress Address of the file object to restore
	 * \param targetPath The path to restore to, if existing files will be created as a subpath otherwise the full path will be restored
	 */
	void RestoreTree(const ObjectAddress& objectAddress, const boost::filesystem::path& targetPath);

	const std::vector<boost::filesystem::path>& GetRestoredPaths() const { return _restoredPaths; }
	const std::vector<boost::filesystem::path>& GetSkippedPaths() const { return _skippedPaths; }
private:
	void Restore(const ObjectAddress& objectAddress, const boost::filesystem::path& targetPath, bool recursive);
	void RestoreFileObject(const FileObject& info, const boost::filesystem::path& targetPath, bool followDirectories);
	bool RestoreBlobToFile(const BlobAddress& blobAddress, const boost::filesystem::path& targetPath) const;
	std::vector<boost::filesystem::path> _restoredPaths;
	std::vector<boost::filesystem::path> _skippedPaths;

	blob::BlobStore& _blobStore;
	blob::BlobInfoRepository& _blobInfoRepository;
	FileObjectRepository& _fileObjectRepository;
	FileRefRepository& _fileRefRepository;
};

}
}
}

