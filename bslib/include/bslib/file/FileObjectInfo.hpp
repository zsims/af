#pragma once

#include "bslib/Address.hpp"

#include <boost/optional.hpp>

#include <string>

namespace af {
namespace bslib {
namespace file {

/**
 * Represents information about a "file"
 */
struct FileObjectInfo
{
	explicit FileObjectInfo(const ObjectAddress& address, const std::string& fullPath, const boost::optional<BlobAddress>& contentBlobAddress)
		: address(address)
		, fullPath(fullPath)
		, contentBlobAddress(contentBlobAddress)
	{
	}

	const ObjectAddress address;
	const std::string fullPath;
	const boost::optional<BlobAddress> contentBlobAddress;

	bool operator==(const FileObjectInfo& rhs) const { return address == rhs.address && fullPath == rhs.fullPath && contentBlobAddress == rhs.contentBlobAddress ; }
};

}
}
}