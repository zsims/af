#pragma once

#include "bslib/file/FileEvent.hpp"
#include "bslib/file/fs/path.hpp"

#include <boost/optional.hpp>

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
		const fs::NativePath& targetPath,
		FileRestoreEventAction action)
		: originalEvent(originalEvent)
		, targetPath(targetPath)
		, action(action)
	{
	}
	
	const FileEvent originalEvent;
	const fs::NativePath targetPath;
	const FileRestoreEventAction action;

	bool operator==(const FileRestoreEvent& rhs) const
	{
		return originalEvent == rhs.originalEvent &&
			targetPath == rhs.targetPath &&
			action == rhs.action;
	}
};

inline std::ostream& operator<<(std::ostream & os, FileRestoreEventAction action)
{
	switch (action)
	{
		case FileRestoreEventAction::Restored:
			return os << "Restored";
		case FileRestoreEventAction::Skipped:
			return os << "Skipped";
		case FileRestoreEventAction::FailedToWriteFile:
			return os << "Failed to write";
		case FileRestoreEventAction::FailedToCreateDirectory:
			return os << "Failed to write directory";
		case FileRestoreEventAction::UnsupportedFileEvent:
			return os << "Unsupported";
	};

	return os << "Unknown";
}

}
}
}