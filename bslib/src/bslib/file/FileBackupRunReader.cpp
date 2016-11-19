#include "bslib/file/FileBackupRunReader.hpp"

#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileBackupRunEventStreamRepository.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <algorithm>
#include <map>

namespace af {
namespace bslib {
namespace file {

FileBackupRunReader::FileBackupRunReader(const FileBackupRunEventStreamRepository& backupRunEventRepository)
	: _backupRunEventRepository(backupRunEventRepository)
{
}

FileBackupRunReader::ResultsPage FileBackupRunReader::GetBackups(unsigned skip, unsigned pageSize) const
{
	const auto events = _backupRunEventRepository.GetPaged(skip, pageSize);
	std::map<Uuid, BackupSummary> summaries;
	
	for (const auto& ev : events)
	{
		auto it = summaries.find(ev.runId);
		if (it == summaries.end())
		{
			BackupSummary summary(ev.runId);
			it = summaries.insert(summaries.begin(), std::make_pair(ev.runId, summary));
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
	std::transform(summaries.begin(), summaries.end(), std::back_inserter(page.backups), [](const auto& pair) { return pair.second; });
	const unsigned summariesSize = static_cast<unsigned>(summaries.size());
	page.nextPageSkip = skip + std::min(summariesSize, pageSize);
	return page;
}

}
}
}

