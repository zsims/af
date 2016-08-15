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

class FileObjectInfoRepository;

/**
 * Adds files to the backup
 */
class FileAdder
{
public:
	FileAdder(blob::BlobStore& blobStore, blob::BlobInfoRepository& blobInfoRepository, FileObjectInfoRepository& fileObjectInfoRepository);
	ObjectAddress Add(const boost::filesystem::path& sourcePath, const std::vector<uint8_t>& content);
private:
	blob::BlobStore& _blobStore;
	blob::BlobInfoRepository& _blobInfoRepository;
	FileObjectInfoRepository& _fileObjectInfoRepository;
};

}
}
}

