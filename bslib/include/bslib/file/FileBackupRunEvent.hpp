#pragma once

#include "bslib/Uuid.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>

namespace af {
namespace bslib {
namespace file {

enum class FileBackupRunEventAction : int
{
	// Change in content or properties
	Started = 0,
	Finished,
};

struct FileBackupRunEvent
{
	FileBackupRunEvent(
		const Uuid& runId,
		const boost::posix_time::ptime& dateTimeUtc,
		FileBackupRunEventAction action)
		: runId(runId)
		, dateTimeUtc(dateTimeUtc)
		, action(action)
	{
	}

	const Uuid runId;
	const boost::posix_time::ptime dateTimeUtc;
	const FileBackupRunEventAction action;

	bool operator==(const FileBackupRunEvent& rhs) const
	{
		return runId == rhs.runId && dateTimeUtc == rhs.dateTimeUtc && action == rhs.action;
	}
};

std::string ToString(FileBackupRunEventAction action);

inline std::ostream& operator<<(std::ostream & os, FileBackupRunEventAction action)
{
	return os << ToString(action);
}

}
}
}