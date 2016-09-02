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
	// Change in content or properties
	ChangedAdded = 0,
	ChangedModified,
	ChangedRemoved,

	// Status events
	FailedToRead,
	Unsupported,
	Unchanged
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
	
	// ANTHONY HEAAAAAAALP see http://stackoverflow.com/questions/14374802/boostoptional-with-const-members
	FileEvent& operator=(const FileEvent&)
	{
		throw "this is not possible";
		return *this;
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