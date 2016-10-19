#pragma once

#include "bslib/Backup.hpp"
#include "bs_daemon_lib/Job.hpp"

#include <boost/noncopyable.hpp>

#include <atomic>
#include <condition_variable>
#include <thread>
#include <memory>
#include <mutex>
#include <queue>

namespace af {
namespace bs_daemon {

/**
 * Schedules and executes backup/restore jobs
 */
class JobExecutor : boost::noncopyable
{
public:
	explicit JobExecutor(bslib::Backup& backup);
	~JobExecutor();

	/**
	 * Queue a job 
	 */
	void Queue(std::unique_ptr<Job> job);

	/**
	 * Stop the job scheduler.
	 * \remarks Running jobs will finish before this returns
	 */
	void Stop();
private:
	void Run();
	void ExecuteJob(std::unique_ptr<Job> job);

	bslib::Backup& _backup;
	std::thread _workThread;
	std::atomic_bool _running;
	std::condition_variable _jobCondition;
	std::queue<std::unique_ptr<Job>> _jobQueue;
	std::mutex _mutex;
};

}
}
