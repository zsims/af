#include "ffs/blob/DirectoryBlobStore.hpp"

#include "ffs/blob/BlobInfo.hpp"
#include "ffs/blob/BlobInfoRepository.hpp"

#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <iostream>
#include <vector>

namespace af {
namespace ffs {
namespace blob {

DirectoryBlobStore::DirectoryBlobStore(std::shared_ptr<BlobInfoRepository> repository, const boost::filesystem::path& rootPath)
	: BlobStore(repository)
	, _rootPath(rootPath)
{
	// Hmm
}

BlobAddress DirectoryBlobStore::CreateBlob(const std::vector<uint8_t>& content)
{
	const auto key = CalculateAddress(content);

	//std::ofstream f(, std::ios::out | std::ofstream::binary);
	//std::copy(myVector.begin(), myVector.end(), std::ostreambuf_iterator<char>(FILE));

	_repository->AddBlob(BlobInfo(key, content.size()));

	return key;
}

}
}
}
