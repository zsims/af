#include "bs_daemon_lib/JobExecutor.hpp"

#include "bslib_test_util/TestBase.hpp"
#include "bslib_test_util/mocks/MockBackup.hpp"
#include "bslib_test_util/mocks/MockUnitOfWork.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <chrono>
#include <mutex>

namespace af {
namespace bs_daemon {
namespace test {

namespace {

class TestJob : public Job
{
public:
	explicit TestJob(std::function<void(bslib::UnitOfWork&)> doIt)
		: _doIt(doIt)
	{
	}

	virtual void Run(bslib::UnitOfWork& unitOfWork) override
	{
		_doIt(unitOfWork);
	}

private:
	const std::function<void(bslib::UnitOfWork&)> _doIt;
};

}

class JobExecutorIntegrationTest : public ::testing::Test
{
protected:
	JobExecutorIntegrationTest()
	{
		ON_CALL(_mockBackup, CreateUnitOfWork())
			.WillByDefault(::testing::Invoke([]() {
			return std::make_unique<bslib_test_util::mocks::MockUnitOfWork>();
		}));
	}
	bslib_test_util::mocks::MockBackup _mockBackup;
};

TEST_F(JobExecutorIntegrationTest, Success)
{
	// Arrange
	af::bs_daemon::JobExecutor jobExecutor(_mockBackup);

	EXPECT_CALL(_mockBackup, CreateUnitOfWork()).Times(::testing::Exactly(1));

	std::timed_mutex mutex;
	mutex.lock();
	bool executed = false;

	// Act
	auto job = std::make_unique<TestJob>([&](auto& uow) -> void {
		executed = true;
		mutex.unlock();
	});
	jobExecutor.Queue(std::move(job));

	// Assert
	std::unique_lock<std::timed_mutex> lock(mutex, std::defer_lock);
	auto result = lock.try_lock_for(std::chrono::seconds(2));
	ASSERT_TRUE(result);
	ASSERT_TRUE(executed);
}

TEST_F(JobExecutorIntegrationTest, FailedJobsAreContained)
{
	// Arrange
	af::bs_daemon::JobExecutor jobExecutor(_mockBackup);

	EXPECT_CALL(_mockBackup, CreateUnitOfWork()).Times(::testing::Exactly(2));

	std::timed_mutex mutex;
	mutex.lock();
	bool executed = false;

	auto job1 = std::make_unique<TestJob>([&](auto& uow) -> void {
		throw std::runtime_error("boom");
	});
	auto job2 = std::make_unique<TestJob>([&](auto& uow) -> void {
		executed = true;
		mutex.unlock();
	});

	// Act
	jobExecutor.Queue(std::move(job1));
	jobExecutor.Queue(std::move(job2));

	// Assert
	std::unique_lock<std::timed_mutex> lock(mutex, std::defer_lock);
	auto result = lock.try_lock_for(std::chrono::seconds(2));
	ASSERT_TRUE(result);
	ASSERT_TRUE(executed);
}

TEST_F(JobExecutorIntegrationTest, ExecutesInOrder)
{
	// Arrange
	af::bs_daemon::JobExecutor jobExecutor(_mockBackup);

	EXPECT_CALL(_mockBackup, CreateUnitOfWork()).Times(::testing::Exactly(3));

	std::timed_mutex mutex;
	mutex.lock();

	int value = 10; // should be ((10+10)*10)+1
	auto job1 = std::make_unique<TestJob>([&](auto& uow) -> void {
		value += 10;
	});
	auto job2 = std::make_unique<TestJob>([&](auto& uow) -> void {
		value *= 10;
	});
	auto job3 = std::make_unique<TestJob>([&](auto& uow) -> void {
		value += 1;
		mutex.unlock();
	});

	// Act
	jobExecutor.Queue(std::move(job1));
	jobExecutor.Queue(std::move(job2));
	jobExecutor.Queue(std::move(job3));

	// Assert
	std::unique_lock<std::timed_mutex> finalLock(mutex, std::defer_lock);
	auto result = finalLock.try_lock_for(std::chrono::seconds(2));
	ASSERT_TRUE(result);
	EXPECT_EQ(201, value);
}

}
}
}
