#include "bs_daemon_lib/JobExecutor.hpp"

#include "bs_daemon_lib/log.hpp"
#include "bs_daemon_lib/Job.hpp"

namespace af {
namespace bs_daemon {

JobExecutor::JobExecutor(bslib::Backup& backup)
	: _backup(backup)
	, _running(true)
{
	_workThread = std::thread(&JobExecutor::Run, this);
}

JobExecutor::~JobExecutor()
{
	Stop();
}

void JobExecutor::Queue(std::unique_ptr<Job> job)
{
	{
		std::lock_guard<std::mutex> guard(_mutex);
		_jobQueue.push(std::move(job));
	}
	_jobCondition.notify_one();
}

void JobExecutor::Stop()
{
	{
		// TODO: flag these jobs as "cancelled"
		std::lock_guard<std::mutex> guard(_mutex);
		std::queue<std::unique_ptr<Job>>().swap(_jobQueue);
	}

	_running = false;
	_jobCondition.notify_all();

	if (_workThread.joinable())
	{
		_workThread.join();
	}
}

void JobExecutor::Run()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_jobCondition.wait(lock, [this] { return !_running || !_jobQueue.empty(); });

		// handle the case where we're stopping
		if (!_running)
		{
			return;
		}

		// handle spurious wakeups
		if (!_jobQueue.empty())
		{
			auto value = std::move(_jobQueue.front());
			_jobQueue.pop();
			ExecuteJob(std::move(value));
		}
	}
}

void JobExecutor::ExecuteJob(std::unique_ptr<Job> job)
{
	try
	{
		// Execute the job
		auto uow = _backup.CreateUnitOfWork();
		job->Run(*uow);
	}
	catch (const std::exception& e)
	{
		BS_DAEMON_LOG_ERROR << "Error while executing job: " << e.what();
	}
	catch (...)
	{
		BS_DAEMON_LOG_ERROR << "Unknown error while executing job";
	}
}

}
}
