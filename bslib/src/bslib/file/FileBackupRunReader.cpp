#include "bslib/file/FileBackupRunReader.hpp"

#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileBackupRunEventStreamRepository.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <map>
#include <vector>

namespace af {
namespace bslib {
namespace file {

FileBackupRunReader::FileBackupRunReader(const FileBackupRunEventStreamRepository& backupRunEventRepository)
	: _backupRunEventRepository(backupRunEventRepository)
{
}

FileBackupRunReader::ResultsPage FileBackupRunReader::GetBackups(unsigned skip, unsigned pageSize) const
{
	// Maintain order of summaries based on the first-seen backup event
	std::vector<Uuid> summaryOrder;
	std::map<Uuid, BackupSummary> summaries;

	const auto events = _backupRunEventRepository.GetPaged(skip, pageSize);
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

	ResultsPage page;
	for (const auto& runId : summaryOrder)
	{
		page.backups.push_back(summaries.at(runId));
	}
	const unsigned summariesSize = static_cast<unsigned>(summaryOrder.size());
	page.nextPageSkip = skip + std::min(summariesSize, pageSize);
	return page;
}

}
}
}

