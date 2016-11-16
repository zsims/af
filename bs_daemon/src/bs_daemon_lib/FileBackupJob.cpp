#include "bs_daemon_lib/FileBackupJob.hpp"

#include "bs_daemon_lib/log.hpp"
#include "bslib/file/FileAdder.hpp"

namespace af {
namespace bs_daemon {

void FileBackupJob::Run(bslib::UnitOfWork& unitOfWork)
{
	auto recorder = unitOfWork.CreateFileBackupRunRecorder();
	auto backupRunId = recorder->Start();
	auto adder = unitOfWork.CreateFileAdder(backupRunId);
	adder->GetEventManager().Subscribe([](const auto& fileEvent) {
		BS_DAEMON_LOG_DEBUG << fileEvent.action << " " << fileEvent.fullPath.ToString();
	});
	adder->Add(_path);
	recorder->Stop(backupRunId);
	unitOfWork.Commit();
}

}
}
