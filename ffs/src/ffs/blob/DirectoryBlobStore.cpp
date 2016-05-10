#include "ffs/blob/DirectoryBlobStore.hpp"

#include "ffs/blob/BlobInfo.hpp"
#include "ffs/blob/BlobInfoRepository.hpp"

namespace af {
namespace ffs {
namespace blob {

DirectoryBlobStore::DirectoryBlobStore(std::shared_ptr<BlobInfoRepository> repository, const std::string& rootPath)
	: BlobStore(repository)
	, _rootPath(rootPath)
{
	// Hmm
}

BlobAddress DirectoryBlobStore::CreateBlob(const std::vector<uint8_t>& content)
{
	const auto key = CalculateAddress(content);
	// TODO create the file on disk under {rootPath}/{key}

	_repository->AddBlob(BlobInfo(key, content.size()));

	return key;
}

}
}
}
