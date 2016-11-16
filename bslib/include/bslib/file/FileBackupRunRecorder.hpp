#pragma once

#include "bslib/EventManager.hpp"
#include "bslib/file/FileBackupRunEvent.hpp"
#include "bslib/Uuid.hpp"

#include <boost/optional.hpp>

#include <functional>
#include <vector>
#include <map>
#include <memory>

namespace af {
namespace bslib {
namespace file {
class FileBackupRunEventStreamRepository;

/**
 * Records backup "runs"
 */
class FileBackupRunRecorder
{
public:
	explicit FileBackupRunRecorder(FileBackupRunEventStreamRepository& backupRunEventRepository);
	const Uuid Start();
	void Stop(const Uuid& runId);
	EventManager<FileBackupRunEvent>& GetEventManager() { return _eventManager; }
private:
	void EmitEvent(const FileBackupRunEvent& backupEvent);
	FileBackupRunEventStreamRepository& _backupRunEventRepository;
	EventManager<FileBackupRunEvent> _eventManager;
};

}
}
}

