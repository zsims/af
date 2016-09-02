#pragma once

#include "bslib/Address.hpp"

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

#include <cstdint>
#include <string>

namespace af {
namespace bslib {
namespace file {

enum class FileEventAction : int
{
	Added = 0,
	Modified,
	Removed
};

struct FileEvent
{
	FileEvent(
		const boost::filesystem::path& fullPath,
		const boost::optional<BlobAddress>& contentBlobAddress, 
		FileEventAction action)
		: fullPath(fullPath)
		, contentBlobAddress(contentBlobAddress)
		, action(action)
	{
	}

	const boost::filesystem::path fullPath;
	const boost::optional<BlobAddress> contentBlobAddress;
	const FileEventAction action;

	bool operator==(const FileEvent& rhs) const
	{
		return fullPath == rhs.fullPath &&
			contentBlobAddress == rhs.contentBlobAddress &&
			action == rhs.action;
	}
};

}
}
}