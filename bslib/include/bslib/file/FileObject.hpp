#pragma once

#include "bslib/Address.hpp"

#include <boost/optional.hpp>

#include <cstdint>
#include <ctime>
#include <random>
#include <string>

namespace af {
namespace bslib {
namespace file {

// File Object Identifier
typedef int64_t foid;

namespace {
std::mt19937 random(static_cast<unsigned>(time(0)));
}

/**
 * Represents information about a file or directory
 */
struct FileObject
{
	FileObject(
		foid id,
		const std::string& fullPath,
		const boost::optional<BlobAddress>& contentBlobAddress,
		const boost::optional<foid>& parentId = boost::none)
		: id(id)
		, fullPath(fullPath)
		, contentBlobAddress(contentBlobAddress)
		, parentId(parentId)
	{
	}

	/**
	 * Creates a new file object from its properties
	 */
	static FileObject CreateFromProperties(
		const std::string& fullPath,
		const boost::optional<BlobAddress>& contentBlobAddress,
		const boost::optional<foid>& parentId)
	{
		return FileObject(random(), fullPath, contentBlobAddress, parentId);
	}

	const foid id;
	const std::string fullPath;
	const boost::optional<BlobAddress> contentBlobAddress;
	const boost::optional<foid> parentId;

	bool operator==(const FileObject& rhs) const
	{
		return id == rhs.id &&
			fullPath == rhs.fullPath &&
			contentBlobAddress == rhs.contentBlobAddress &&
			parentId == rhs.parentId;
	}
};

}
}
}