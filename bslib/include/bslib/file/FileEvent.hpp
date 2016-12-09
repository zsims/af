#pragma once

#include "bslib/blob/Address.hpp"
#include "bslib/file/FileType.hpp"
#include "bslib/file/fs/path.hpp"
#include "bslib/Uuid.hpp"

#include <boost/optional.hpp>

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
		const Uuid& backupRunId,
		const fs::NativePath& fullPath,
		FileType type,
		const boost::optional<blob::Address>& contentBlobAddress, 
		FileEventAction action)
		: backupRunId(backupRunId)
		, fullPath(fullPath)
		, type(type)
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

	const Uuid backupRunId;
	const fs::NativePath fullPath;
	const FileType type;
	const boost::optional<blob::Address> contentBlobAddress;
	const FileEventAction action;

	bool operator==(const FileEvent& rhs) const
	{
		return backupRunId == rhs.backupRunId &&
			fullPath == rhs.fullPath &&
			type == rhs.type &&
			contentBlobAddress == rhs.contentBlobAddress &&
			action == rhs.action;
	}
};

struct DirectoryEvent : public FileEvent
{
	DirectoryEvent(
		const Uuid& backupRunId,
		const fs::NativePath& fullPath,
		FileEventAction action)
		: FileEvent(backupRunId, fullPath, FileType::Directory, boost::none, action)
	{
	}
};

struct RegularFileEvent : public FileEvent
{
	RegularFileEvent(
		const Uuid& backupRunId,
		const fs::NativePath& fullPath,
		const boost::optional<blob::Address>& contentBlobAddress, 
		FileEventAction action)
		: FileEvent(backupRunId, fullPath, FileType::RegularFile, contentBlobAddress, action)
	{
	}
};

std::string ToString(FileEventAction action);

inline std::ostream& operator<<(std::ostream & os, FileEventAction action)
{
	return os << ToString(action);
}

}
}
}