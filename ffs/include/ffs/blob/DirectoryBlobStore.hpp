#pragma once

#include "ffs/blob/BlobStore.hpp"

#include <string>

namespace af {
namespace ffs {
namespace blob {

/**
 * Manages blobs with-in a directory
 */
class DirectoryBlobStore : public BlobStore
{
public:
	DirectoryBlobStore(std::shared_ptr<BlobInfoRepository> repository, const std::string& rootPath);

	BlobAddress CreateBlob(const std::vector<uint8_t>& content) override;

private:
	const std::string _rootPath;
};

}
}
}
