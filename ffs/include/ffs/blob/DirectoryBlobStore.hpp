#pragma once

#include "ffs/blob/BlobStore.hpp"

#include <boost/filesystem/path.hpp>

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
	explicit DirectoryBlobStore(const boost::filesystem::path& rootPath);
	void CreateBlob(const BlobAddress& address, const std::vector<uint8_t>& content) override;
	std::vector<uint8_t> GetBlob(const BlobAddress& address) override;
private:
	const boost::filesystem::path _rootPath;
};

}
}
}
