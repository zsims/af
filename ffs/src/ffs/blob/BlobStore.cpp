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

}
}
}

