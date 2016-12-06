#pragma once

#include "bslib/file/FileEvent.hpp"

#include <boost/optional.hpp>

namespace af {
namespace bslib {
namespace file {

struct VirtualFile
{
	VirtualFile(const fs::NativePath& fullPath, FileType type)
		: fullPath(fullPath)
		, type(type)
	{
	}
	const fs::NativePath fullPath;
	const FileType type;
	boost::optional<FileEvent> matchedFileEvent;
};

}
}
}
