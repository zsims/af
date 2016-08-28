#pragma once

#include "bslib/Address.hpp"
#include "bslib/file/FileObject.hpp"

#include <string>

namespace af {
namespace bslib {
namespace file {

/**
 * Represents a reference to a file object
 */
struct FileRef
{
	FileRef(const std::string& fullPath, foid fileObjectId)
		: fullPath(fullPath)
		, fileObjectId(fileObjectId)
	{
	}

	const std::string fullPath;
	const foid fileObjectId;

	bool operator==(const FileRef& rhs) const
	{
		return fullPath == rhs.fullPath && fileObjectId == rhs.fileObjectId;
	}
};

}
}
}