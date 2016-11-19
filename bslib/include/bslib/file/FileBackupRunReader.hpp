#pragma once

#include "bslib/file/FileBackupRunEvent.hpp"
#include "bslib/Uuid.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <vector>

namespace af {
namespace bslib {
namespace file {
class FileBackupRunEventStreamRepository;

class FileBackupRunReader
{
public:
	struct BackupSummary
	{
		explicit BackupSummary(const Uuid& runId)
			: runId(runId)
		{
		}

		Uuid runId;
		boost::posix_time::ptime startedUtc;
		boost::optional<boost::posix_time::ptime> finishedUtc;
	};

	struct ResultsPage
	{
		unsigned nextPageSkip;
		std::vector<BackupSummary> backups;
	};

	explicit FileBackupRunReader(const FileBackupRunEventStreamRepository& backupRunEventRepository);

	/**
	 * Gets a page of backups
	 */
	ResultsPage GetBackups(unsigned skip, unsigned pageSize) const;
private:
	const FileBackupRunEventStreamRepository& _backupRunEventRepository;
};

}
}
}

