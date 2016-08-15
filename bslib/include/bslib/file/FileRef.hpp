#pragma once

#include "bslib/Address.hpp"

#include <string>

namespace af {
namespace bslib {
namespace file {

/**
 * Represents a reference to a file object
 */
struct FileRef
{
	FileRef(const std::string& fullPath, const ObjectAddress& fileObjectAddress)
		: fullPath(fullPath)
		, fileObjectAddress(fileObjectAddress)
	{
	}

	const std::string fullPath;
	const ObjectAddress fileObjectAddress;

	bool operator==(const FileRef& rhs) const
	{
		return fullPath == rhs.fullPath && fileObjectAddress == rhs.fileObjectAddress;
	}
};

}
}
}