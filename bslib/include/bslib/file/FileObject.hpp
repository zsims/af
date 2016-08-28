#pragma once

#include "bslib/Address.hpp"

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

#include <cstdint>
#include <string>

namespace af {
namespace bslib {
namespace file {

// File Object Identifier
typedef int64_t foid;

/**
 * Represents information about a file or directory
 */
struct FileObject
{
	FileObject(
		foid id,
		const boost::filesystem::path& fullPath,
		const boost::optional<BlobAddress>& contentBlobAddress,
		const boost::optional<foid>& parentId = boost::none)
		: id(id)
		, fullPath(fullPath)
		, contentBlobAddress(contentBlobAddress)
		, parentId(parentId)
	{
	}

	const foid id;
	const boost::filesystem::path fullPath;
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