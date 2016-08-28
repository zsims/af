#pragma once

#include "bslib/Address.hpp"
#include "bslib/file/FileObject.hpp"

#include <boost/filesystem/path.hpp>

#include <string>

namespace af {
namespace bslib {
namespace file {

/**
 * Represents a reference to a file object
 */
struct FileRef
{
	FileRef(const boost::filesystem::path& fullPath, foid fileObjectId)
		: fullPath(fullPath)
		, fileObjectId(fileObjectId)
	{
	}

	const boost::filesystem::path fullPath;
	const foid fileObjectId;

	bool operator==(const FileRef& rhs) const
	{
		return fullPath == rhs.fullPath && fileObjectId == rhs.fileObjectId;
	}
};

}
}
}