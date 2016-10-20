#include "bs_daemon_lib/FileBackupJob.hpp"

#include "bs_daemon_lib/log.hpp"
#include "bslib/file/FileAdder.hpp"

namespace af {
namespace bs_daemon {

void FileBackupJob::Run(bslib::UnitOfWork& unitOfWork)
{
	auto adder = unitOfWork.CreateFileAdder();
	adder->GetEventManager().Subscribe([](const auto& fileEvent) {
		BS_DAEMON_LOG_DEBUG << fileEvent.action << " " << fileEvent.fullPath.ToString();
	});
	adder->Add(_path);
	unitOfWork.Commit();
}

}
}
