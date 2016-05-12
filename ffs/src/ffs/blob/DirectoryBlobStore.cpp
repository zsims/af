#include "ffs/blob/DirectoryBlobStore.hpp"

#include "ffs/blob/BlobInfo.hpp"
#include "ffs/blob/BlobInfoRepository.hpp"
#include "ffs/blob/exceptions.hpp"

#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <fstream>
#include <iterator>
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
	const auto address = CalculateAddress(content);
	const auto stringAddress = address.ToString();

	// Save the blob
	const auto blobPath = _rootPath / stringAddress;
	std::ofstream f(blobPath.string(), std::ios::out | std::ofstream::binary);
	std::copy(content.begin(), content.end(), std::ostreambuf_iterator<char>(f));

	// Track it
	_repository->AddBlob(BlobInfo(address, content.size()));

	return address;
}

std::vector<uint8_t> DirectoryBlobStore::GetBlob(const BlobAddress& address)
{
	std::vector<uint8_t> result;
	const auto stringAddress = address.ToString();
	const auto blobPath = _rootPath / stringAddress;

	// TODO does it really need to be tracked?
	std::ifstream f(blobPath.string(), std::ios::in | std::ifstream::binary);
	if (f.fail())
	{
		throw BlobReadException(address);
	}

	while (!f.eof())
	{
		char buffer[4096];
		f.read(buffer, sizeof(buffer));
		result.insert(result.end(), buffer, buffer + f.gcount());
	}

	return result;
}

}
}
}
