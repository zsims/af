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
class FileEventStreamRepository;
struct FileBackupRunSearchCriteria;

class FileBackupRunReader
{
public:
	struct BackupSummary
	{
		explicit BackupSummary(const Uuid& runId)
			: runId(runId)
			, modifiedFilesCount(0)
			, totalSizeBytes(0)
		{
		}

		Uuid runId;
		uint64_t totalSizeBytes;
		unsigned modifiedFilesCount;
		boost::posix_time::ptime startedUtc;
		boost::optional<boost::posix_time::ptime> finishedUtc;
	};

	struct ResultsPage
	{
		ResultsPage()
			: totalBackups(0)
			, nextPageSkip(0)
		{

		}
		unsigned totalBackups;
		unsigned nextPageSkip;
		std::vector<BackupSummary> backups;
	};

	FileBackupRunReader(
		const FileBackupRunEventStreamRepository& backupRunEventRepository,
		const FileEventStreamRepository& fileEventStreamRepository);

	/**
	 * Gets searches for a list of backups
	 */
	ResultsPage Search(const FileBackupRunSearchCriteria& criteria) const;
private:
	const FileBackupRunEventStreamRepository& _backupRunEventRepository;
	const FileEventStreamRepository& _fileEventStreamRepository;
};

}
}
}

