#include "ffs/blob/BlobStore.hpp"

#include "ffs/blob/BlobInfo.hpp"
#include "ffs/blob/BlobInfoRepository.hpp"

namespace af {
namespace ffs {
namespace blob {

BlobStore::BlobStore(std::shared_ptr<BlobInfoRepository> repository)
	: _repository(repository)
{
	// Hmm
}

BlobAddress BlobStore::CalculateAddress(const std::vector<uint8_t>& content)
{
	// Do SHA1 on the content
	return BlobAddress("cf23df2207d99a74fbe169e3eba035e633b65d94");
}

}
}
}

