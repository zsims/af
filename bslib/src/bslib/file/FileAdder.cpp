#include "bslib/file/FileAdder.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileObjectInfoRepository.hpp"

#include <boost/filesystem.hpp>

#include <vector>

namespace af {
namespace bslib {
namespace file {

FileAdder::FileAdder(blob::BlobStore& blobStore, blob::BlobInfoRepository& blobInfoRepository, FileObjectInfoRepository& fileObjectInfoRepository)
	: _blobStore(blobStore)
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

	const auto info = FileObjectInfo::CreateFromProperties(sourcePath.string(), blobAddress, boost::none);
	_fileObjectInfoRepository.AddObject(info);
	return info.address;
}


}
}
}

