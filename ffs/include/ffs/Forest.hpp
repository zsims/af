#pragma once

#include <memory>
#include <string>

namespace af {
namespace ffs {

namespace blob {
class BlobInfoRepository;
}

class Forest
{
public:
	explicit Forest(const std::string& forestFile);

	std::shared_ptr<blob::BlobInfoRepository> GetBlobInfoRepository() const { return _blobInfoRepository; }
private:
	const std::string _forestFile;
	std::shared_ptr<blob::BlobInfoRepository> _blobInfoRepository;
};

}
}