#pragma once

#include "bslib/Address.hpp"

#include <boost/optional.hpp>

#include <string>

namespace af {
namespace bslib {
namespace file {

/**
 * Represents information about a file or directory
 */
struct FileObjectInfo
{
	explicit FileObjectInfo(
		const ObjectAddress& address,
		const std::string& fullPath,
		const boost::optional<BlobAddress>& contentBlobAddress,
		const boost::optional<ObjectAddress>& parentAddress = boost::none)
		: address(address)
		, fullPath(fullPath)
		, contentBlobAddress(contentBlobAddress)
		, parentAddress(parentAddress)
	{
	}

	const ObjectAddress address;
	const std::string fullPath;
	const boost::optional<BlobAddress> contentBlobAddress;
	const boost::optional<ObjectAddress> parentAddress;

	bool operator==(const FileObjectInfo& rhs) const
	{
		return address == rhs.address &&
			fullPath == rhs.fullPath &&
			contentBlobAddress == rhs.contentBlobAddress &&
			parentAddress == rhs.parentAddress;
	}
};

}
}
}