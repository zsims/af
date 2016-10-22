#pragma once

#include "bs_daemon_lib/Job.hpp"
#include "bslib/unicode.hpp"

namespace af {
namespace bs_daemon {

class FileBackupJob : public Job
{
public:
	explicit FileBackupJob(const bslib::UTF8String& path)
		: _path(path)
	{
	}
	virtual ~FileBackupJob() { }
	void Run(bslib::UnitOfWork& unitOfWork) override;
private:
	const bslib::UTF8String _path;
};

}
}
