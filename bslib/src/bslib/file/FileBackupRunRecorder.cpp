#include "bslib/file/FileBackupRunRecorder.hpp"

#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileBackupRunEventStreamRepository.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <vector>
#include <map>

namespace af {
namespace bslib {
namespace file {

FileBackupRunRecorder::FileBackupRunRecorder(FileBackupRunEventStreamRepository& backupRunEventRepository)
	: _backupRunEventRepository(backupRunEventRepository)
{
}

const Uuid FileBackupRunRecorder::Start()
{
	const auto runId = Uuid::Create();
	EmitEvent(FileBackupRunEvent(runId, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Started));
	return runId;
}

void FileBackupRunRecorder::Stop(const Uuid& runId)
{
	EmitEvent(FileBackupRunEvent(runId, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Finished));
}

void FileBackupRunRecorder::EmitEvent(const FileBackupRunEvent& backupEvent)
{
	_backupRunEventRepository.AddEvent(backupEvent);
	_eventManager.Publish(backupEvent);
}

}
}
}

