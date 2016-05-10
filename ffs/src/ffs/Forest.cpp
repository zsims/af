#include "ffs/Forest.hpp"

#include "ffs/blob/BlobInfoRepository.hpp"

namespace af {
namespace ffs {

Forest::Forest(const std::string& forestFile)
	: _forestFile(forestFile)
	, _blobInfoRepository(new blob::BlobInfoRepository())
{
}

}
}

