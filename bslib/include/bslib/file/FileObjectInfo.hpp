#pragma once

#include "bslib/Address.hpp"

#include <boost/optional.hpp>

#include <ctime>
#include <random>
#include <string>

namespace af {
namespace bslib {
namespace file {
namespace {
std::mt19937 random(static_cast<unsigned>(time(0)));
}

/**
 * Represents information about a file or directory
 */
struct FileObjectInfo
{
	FileObjectInfo(
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

	/**
	 * Creates a new file object from its properties, calculating the address on the fly.
	 */
	static FileObjectInfo CreateFromProperties(
		const std::string& fullPath,
		const boost::optional<BlobAddress>& contentBlobAddress,
		const boost::optional<ObjectAddress>& parentAddress)
	{
		auto r = [&]() {
			return static_cast<uint8_t>(random());
		};
		const ObjectAddress address(binary_address{
			r(), r(), r(), r(), r(),
			r(), r(), r(), r(), r(),
			r(), r(), r(), r(), r(),
			r(), r(), r(), r(), r()
		});

		return FileObjectInfo(address, fullPath, contentBlobAddress, parentAddress);
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