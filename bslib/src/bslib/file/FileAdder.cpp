#include "bslib/file/FileAdder.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/FileObjectInfoRepository.hpp"

#include <boost/filesystem/path.hpp>

#include <vector>

namespace af {
namespace bslib {
namespace file {

FileAdder::FileAdder(blob::BlobStore& blobStore, blob::BlobInfoRepository& blobInfoRepository, FileObjectInfoRepository& fileObjectInfoRepository)
	: _random(static_cast<unsigned>(time(0)))
	, _blobStore(blobStore)
	, _blobInfoRepository(blobInfoRepository)
	, _fileObjectInfoRepository(fileObjectInfoRepository)
{
}

ObjectAddress FileAdder::Add(const boost::filesystem::path& sourcePath, const std::vector<uint8_t>& content)
{
	const auto blobAddress = BlobAddress::CalculateFromContent(content);
	const auto existingBlob = _blobInfoRepository.FindBlob(blobAddress);
	if (!existingBlob)
	{
		_blobStore.CreateBlob(blobAddress, content);
		_blobInfoRepository.AddBlob(blob::BlobInfo(blobAddress, content.size()));
	}

	// TODO: should this come from a combination of the properties + blob address?
	auto r = [this]() {
		return static_cast<uint8_t>(_random());
	};
	// generate a new random address
	ObjectAddress address(binary_address{
		r(), r(), r(), r(), r(),
		r(), r(), r(), r(), r(),
		r(), r(), r(), r(), r(),
		r(), r(), r(), r(), r()
	});
	file::FileObjectInfo info(address, sourcePath.string(), blobAddress);
	_fileObjectInfoRepository.AddObject(info);
	return address;
}

}
}
}

