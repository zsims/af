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

const std::string DirectoryBlobStore::TYPE = "directory";

DirectoryBlobStore::DirectoryBlobStore(const boost::filesystem::path& rootPath)
	: DirectoryBlobStore(Uuid::Create(), rootPath)
{
}

DirectoryBlobStore::DirectoryBlobStore(const Uuid& id, const nlohmann::json& settings)
	: DirectoryBlobStore(id, boost::filesystem::path(settings.at("path")))
{
}

DirectoryBlobStore::DirectoryBlobStore(const Uuid& id, const boost::filesystem::path& rootPath)
	: _rootPath(rootPath)
	, _id(id)
{
	boost::system::error_code ec;
	boost::filesystem::create_directories(_rootPath, ec);
	if (ec)
	{
		throw DirectoryBlobCreationFailed("Failed to create root path", _rootPath, ec);
	}
}

void DirectoryBlobStore::CreateBlob(const Address& address, const std::vector<uint8_t>& content)
{
	const auto stringAddress = address.ToString();

	// Save the blob
	const auto blobPath = _rootPath / stringAddress;
	std::ofstream f(blobPath.string(), std::ios::out | std::ofstream::binary);
	if (!f)
	{
		throw CreateBlobFailed("Failed to write blob file", blobPath);
	}
	std::copy(content.begin(), content.end(), std::ostreambuf_iterator<char>(f));
}

void DirectoryBlobStore::CreateNamedBlob(const UTF8String& name, const boost::filesystem::path& sourcePath)
{
	const auto blobPath = _rootPath / boost::filesystem::path(UTF8ToWideString(name));
	boost::system::error_code ec;
	boost::filesystem::copy_file(sourcePath, blobPath, boost::filesystem::copy_option::overwrite_if_exists, ec);
	if (ec)
	{
		throw CreateBlobFailed("Failed to write named blob file", blobPath, ec);
	}
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

nlohmann::json DirectoryBlobStore::ConvertToJson() const
{
	nlohmann::json result;
	result["path"] = _rootPath.string();
	return result;
}

}
}
}
