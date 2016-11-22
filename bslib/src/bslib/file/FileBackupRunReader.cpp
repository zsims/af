#include "bslib/file/FileBackupRunReader.hpp"

#include "bslib/log.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileBackupRunEventStreamRepository.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <map>
#include <vector>
#include <set>

namespace af {
namespace bslib {
namespace file {

namespace {
const std::set<FileEventAction> MODIFIED_ACTIONS{FileEventAction::ChangedAdded, FileEventAction::ChangedModified};
}

FileBackupRunReader::FileBackupRunReader(
	const FileBackupRunEventStreamRepository& backupRunEventRepository,
	const FileEventStreamRepository& fileEventStreamRepository)
	: _backupRunEventRepository(backupRunEventRepository)
	, _fileEventStreamRepository(fileEventStreamRepository)
{
}

FileBackupRunReader::ResultsPage FileBackupRunReader::Search(const FileBackupRunSearchCriteria& criteria) const
{
	// Maintain order of summaries based on the first-seen backup event
	std::vector<Uuid> summaryOrder;
	std::map<Uuid, BackupSummary> summaries;

	const auto events = _backupRunEventRepository.SearchByRun(criteria);
	for (const auto& ev : events)
	{
		auto it = summaries.find(ev.runId);
		if (it == summaries.end())
		{
			BackupSummary summary(ev.runId);
			it = summaries.insert(summaries.begin(), std::make_pair(ev.runId, summary));
			summaryOrder.push_back(ev.runId);
		}

		auto& foundSummary = it->second;
		switch (ev.action)
		{
			case FileBackupRunEventAction::Started:
				foundSummary.startedUtc = ev.dateTimeUtc;
				break;
			case FileBackupRunEventAction::Finished:
				foundSummary.finishedUtc= ev.dateTimeUtc;
				break;
		}
	}

	const auto allStats = _fileEventStreamRepository.GetStatisticsByRunId(summaryOrder, MODIFIED_ACTIONS);
	ResultsPage page;
	for (const auto& runId : summaryOrder)
	{
		auto& summary = summaries.at(runId);
		const auto statsIt = allStats.find(runId);
		if (statsIt != allStats.end())
		{
			summary.modifiedFilesCount = statsIt->second.matchingEvents;
			summary.totalSizeBytes = statsIt->second.matchingSizeBytes;
		}
		else
		{
			BSLIB_LOG_WARNING << "Failed to calculate statistics for run " << runId;
		}
		page.backups.push_back(summary);
	}
	const unsigned summariesSize = static_cast<unsigned>(summaryOrder.size());
	page.nextPageSkip = criteria.skipRuns + std::min(summariesSize, criteria.uniqueRunLimit);
	page.totalBackups = _backupRunEventRepository.GetBackupCount();
	return page;
}

}
}
}

