#include "bslib/file/FileAdder.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileRefRepository.hpp"
#include "bslib/file/FileObjectRepository.hpp"

#include <boost/filesystem.hpp>

#include <vector>

namespace af {
namespace bslib {
namespace file {

FileAdder::FileAdder(
	blob::BlobStore& blobStore,
	blob::BlobInfoRepository& blobInfoRepository,
	FileObjectRepository& fileObjectRepository,
	FileRefRepository& fileRefRepository)
	: _blobStore(blobStore)
	, _blobInfoRepository(blobInfoRepository)
	, _fileObjectRepository(fileObjectRepository)
	, _fileRefRepository(fileRefRepository)
{
}

foid FileAdder::Add(const boost::filesystem::path& sourcePath, const std::vector<uint8_t>& content)
{
	const auto blobAddress = BlobAddress::CalculateFromContent(content);
	const auto existingBlob = _blobInfoRepository.FindBlob(blobAddress);
	if (!existingBlob)
	{
		_blobStore.CreateBlob(blobAddress, content);
		_blobInfoRepository.AddBlob(blob::BlobInfo(blobAddress, content.size()));
	}

	const auto id = _fileObjectRepository.AddObject(sourcePath, blobAddress, boost::none);
	_fileRefRepository.SetReference(sourcePath, id);
	return id;
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

	// Check this is a supported path type
	if (!boost::filesystem::is_regular_file(sourcePath) && !boost::filesystem::is_directory(sourcePath))
	{
		throw SourcePathNotSupportedException(sourcePath.string());
	}

	AddChild(sourcePath, boost::none);
}

void FileAdder::AddChild(const boost::filesystem::path& sourcePath, const boost::optional<foid>& parentId)
{
	if (boost::filesystem::is_regular_file(sourcePath))
	{
		AddFile(sourcePath, parentId);
	}
	else if (boost::filesystem::is_directory(sourcePath))
	{
		auto nextAddress = AddDirectory(sourcePath, parentId);
		boost::filesystem::recursive_directory_iterator itr(sourcePath);
		for (const auto& path : itr)
		{
			AddChild(path, nextAddress);
		}
	}
	else
	{
		_skippedPaths.push_back(sourcePath);
	}
}

void FileAdder::AddFile(const boost::filesystem::path& sourcePath, const boost::optional<foid>& parentId)
{
	const auto blobAddress = SaveFileContents(sourcePath);

	if (!blobAddress)
	{
		// Expected some contents as this is a file
		_skippedPaths.push_back(sourcePath);
		return;
	}

	const auto id = _fileObjectRepository.AddObject(sourcePath, blobAddress, parentId);
	_fileRefRepository.SetReference(sourcePath, id);
	_addedPaths.push_back(sourcePath);
}

foid FileAdder::AddDirectory(const boost::filesystem::path& sourcePath, const boost::optional<foid>& parentId)
{
	const auto id = _fileObjectRepository.AddObject(sourcePath, boost::none, parentId);
	_fileRefRepository.SetReference(sourcePath, id);
	_addedPaths.push_back(sourcePath);
	return id;
}

}
}
}

