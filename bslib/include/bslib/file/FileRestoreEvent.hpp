#pragma once

#include "bslib/Address.hpp"
#include "bslib/file/FileEvent.hpp"

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

#include <cstdint>
#include <string>

namespace af {
namespace bslib {
namespace file {

enum class FileRestoreEventAction : int
{
	Restored = 0,
	Skipped,
	FailedToWriteFile,
	FailedToCreateDirectory,
	UnsupportedFileEvent
};

struct FileRestoreEvent
{
	FileRestoreEvent(
		const FileEvent& originalEvent,
		const boost::filesystem::path& targetPath,
		FileRestoreEventAction action)
		: originalEvent(originalEvent)
		, targetPath(targetPath)
		, action(action)
	{
	}
	
	const FileEvent originalEvent;
	const boost::filesystem::path targetPath;
	const FileRestoreEventAction action;

	bool operator==(const FileRestoreEvent& rhs) const
	{
		return originalEvent == rhs.originalEvent &&
			targetPath == rhs.targetPath &&
			action == rhs.action;
	}
};

}
}
}