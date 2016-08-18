#include "bslib/file/FileAdder.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileRefRepository.hpp"
#include "bslib/file/FileObjectInfoRepository.hpp"

#include <boost/filesystem.hpp>

#include <vector>

namespace af {
namespace bslib {
namespace file {

FileAdder::FileAdder(
	blob::BlobStore& blobStore,
	blob::BlobInfoRepository& blobInfoRepository,
	FileObjectInfoRepository& fileObjectInfoRepository,
	FileRefRepository& fileRefRepository)
	: _blobStore(blobStore)
	, _blobInfoRepository(blobInfoRepository)
	, _fileObjectInfoRepository(fileObjectInfoRepository)
	, _fileRefRepository(fileRefRepository)
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
	_fileRefRepository.SetReference(FileRef(sourcePath.string(), info.address));
	return info.address;
}

boost::optional<BlobAddress> FileAdder::SaveFileContents(const boost::filesystem::path& sourcePath)
{
	// TODO: ffs should really support files so they don't have to be read into memory. Or at least streaming...
	std::ifstream file(sourcePath.string(), std::ios::binary | std::ios::in);

	if (!file)
	{
		return boost::none;
	}

	const auto content = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	const auto blobAddress = BlobAddress::CalculateFromContent(content);
	const auto existingBlob = _blobInfoRepository.FindBlob(blobAddress);
	if (!existingBlob)
	{
		_blobStore.CreateBlob(blobAddress, content);
		_blobInfoRepository.AddBlob(blob::BlobInfo(blobAddress, content.size()));
	}
	return blobAddress;
}

void FileAdder::Add(const boost::filesystem::path& sourcePath)
{
	if (!boost::filesystem::exists(sourcePath))
	{
		throw PathNotFoundException(sourcePath.string());
	}

	if (boost::filesystem::is_regular_file(sourcePath))
	{
		AddFile(sourcePath);
	}
	else
	{
		throw SourcePathNotSupportedException(sourcePath.string());
	}
}

void FileAdder::AddFile(const boost::filesystem::path& sourcePath)
{
	const auto blobAddress = SaveFileContents(sourcePath);

	if (!blobAddress)
	{
		// Expected some contents as this is a file
		_skippedPaths.push_back(sourcePath);
		return;
	}

	const auto info = FileObjectInfo::CreateFromProperties(sourcePath.string(), blobAddress, boost::none);
	_fileObjectInfoRepository.AddObject(info);
	_fileRefRepository.SetReference(FileRef(sourcePath.string(), info.address));
	_addedPaths.push_back(sourcePath);
}

}
}
}

