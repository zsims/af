#pragma once

#include "bslib/Address.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <vector>

namespace af {
namespace bslib {
namespace blob {
class BlobInfoRepository;
class BlobStore;
}
namespace file {

class FileRefRepository;
class FileObjectInfoRepository;

/**
 * Adds files to the backup
 */
class FileAdder
{
public:
	FileAdder(
		blob::BlobStore& blobStore,
		blob::BlobInfoRepository& blobInfoRepository,
		FileObjectInfoRepository& fileObjectInfoRepository,
		FileRefRepository& fileRefRepository);
	ObjectAddress Add(const boost::filesystem::path& sourcePath, const std::vector<uint8_t>& content);

	/**
	* Adds the contents of the given file or directory to the attached backup
	* \exception PathNotFoundException File or directory doesn't exist at the given path
	* \exception SourcePathNotSupportedException The given source isn't a file or directory
	*/
	void Add(const boost::filesystem::path& sourcePath);
	const std::vector<boost::filesystem::path>& GetAddedPaths() const { return _addedPaths; }
	const std::vector<boost::filesystem::path>& GetSkippedPaths() const { return _skippedPaths; }
private:
	void AddChild(const boost::filesystem::path& sourcePath, const boost::optional<ObjectAddress>& parentAddress);
	boost::optional<BlobAddress> SaveFileContents(const boost::filesystem::path& sourcePath);
	void AddFile(const boost::filesystem::path& sourcePath, const boost::optional<ObjectAddress>& parentAddress);
	ObjectAddress AddDirectory(const boost::filesystem::path& sourcePath, const boost::optional<ObjectAddress>& parentAddress);

	std::vector<boost::filesystem::path> _addedPaths;
	std::vector<boost::filesystem::path> _skippedPaths;

	blob::BlobStore& _blobStore;
	blob::BlobInfoRepository& _blobInfoRepository;
	FileObjectInfoRepository& _fileObjectInfoRepository;
	FileRefRepository& _fileRefRepository;
};

}
}
}

