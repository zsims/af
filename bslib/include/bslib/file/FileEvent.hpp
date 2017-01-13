#pragma once

#include "bslib/blob/Address.hpp"
#include "bslib/file/FileType.hpp"
#include "bslib/file/fs/path.hpp"
#include "bslib/Uuid.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
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
		FileEventAction action,
		const boost::posix_time::ptime& dateTimeUtc = boost::posix_time::second_clock::universal_time())
		: backupRunId(backupRunId)
		, fullPath(fullPath)
		, type(type)
		, contentBlobAddress(contentBlobAddress)
		, dateTimeUtc(dateTimeUtc)
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
	const boost::posix_time::ptime dateTimeUtc;
	const FileEventAction action;

	bool operator==(const FileEvent& rhs) const
	{
		// Note this purposefully ignores dates as this is really only used for testing
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
		FileEventAction action,
		const boost::posix_time::ptime& dateTimeUtc = boost::posix_time::second_clock::universal_time())
		: FileEvent(backupRunId, fullPath, FileType::Directory, boost::none, action, dateTimeUtc)
	{
	}
};

struct RegularFileEvent : public FileEvent
{
	RegularFileEvent(
		const Uuid& backupRunId,
		const fs::NativePath& fullPath,
		const boost::optional<blob::Address>& contentBlobAddress, 
		FileEventAction action,
		const boost::posix_time::ptime& dateTimeUtc = boost::posix_time::second_clock::universal_time())
		: FileEvent(backupRunId, fullPath, FileType::RegularFile, contentBlobAddress, action, dateTimeUtc)
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