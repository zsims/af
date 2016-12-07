#pragma once

#include "bslib/file/FileEvent.hpp"

#include <boost/optional.hpp>

namespace af {
namespace bslib {
namespace file {

struct VirtualFile
{
	VirtualFile(int64_t pathId, const fs::NativePath& fullPath, FileType type)
		: pathId(pathId)
		, fullPath(fullPath)
		, type(type)
		, containsMatchingEvent(false)
	{
	}
	const fs::NativePath fullPath;
	const int64_t pathId;
	const FileType type;

	// whether there is a matching event on or under this path
	const bool containsMatchingEvent;

	// the match on this path
	boost::optional<FileEvent> matchedFileEvent;
};

}
}
}
