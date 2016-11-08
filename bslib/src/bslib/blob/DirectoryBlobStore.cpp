#include "bslib/blob/DirectoryBlobStore.hpp"

#include "bslib/blob/BlobInfo.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/blob/exceptions.hpp"

#include <boost/filesystem.hpp>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <vector>

namespace af {
namespace bslib {
namespace blob {

DirectoryBlobStore::DirectoryBlobStore(const boost::filesystem::path& rootPath)
	: _rootPath(rootPath)
{
}

void DirectoryBlobStore::CreateBlob(const Address& address, const std::vector<uint8_t>& content)
{
	const auto stringAddress = address.ToString();

	// Save the blob
	const auto blobPath = _rootPath / stringAddress;
	std::ofstream f(blobPath.string(), std::ios::out | std::ofstream::binary);
	std::copy(content.begin(), content.end(), std::ostreambuf_iterator<char>(f));
}

void DirectoryBlobStore::CreateNamedBlob(const UTF8String& name, const boost::filesystem::path& sourcePath)
{
	const auto blobPath = _rootPath / boost::filesystem::path(UTF8ToWideString(name));
	if (boost::filesystem::exists(blobPath))
	{
		boost::filesystem::remove(blobPath);
	}
	boost::filesystem::copy_file(sourcePath, blobPath);
}

std::vector<uint8_t> DirectoryBlobStore::GetBlob(const Address& address) const
{
	std::vector<uint8_t> result;
	const auto stringAddress = address.ToString();
	const auto blobPath = _rootPath / stringAddress;

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
