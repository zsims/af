#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileBackupRunRecorder.hpp"
#include "bslib_test_util/TestBase.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <functional>
#include <memory>

namespace af {
namespace bslib {
namespace file {
namespace test {

class FileBackupRunRecorderIntegrationTest : public bslib_test_util::TestBase
{
protected:
	FileBackupRunRecorderIntegrationTest()
	{
		_testBackup.Create();
		_uow = _testBackup.GetBackup().CreateUnitOfWork();
	}

	std::unique_ptr<UnitOfWork> _uow;
};

TEST_F(FileBackupRunRecorderIntegrationTest, Start_RecordsStartEvent)
{
	// Arrange
	auto recorder = _uow->CreateFileBackupRunRecorder();

	std::vector<FileBackupRunEvent> emittedEvents;
	recorder->GetEventManager().Subscribe([&](const auto& backupRunEvent) {
		emittedEvents.push_back(backupRunEvent);
	});

	// Act
	recorder->Start();

	// Assert
	EXPECT_EQ(1, emittedEvents.size());
	const auto first = *(emittedEvents.begin());
	EXPECT_EQ(FileBackupRunEventAction::Started, first.action);
}

TEST_F(FileBackupRunRecorderIntegrationTest, Stop_RecordsStopEvent)
{
	// Arrange
	auto recorder = _uow->CreateFileBackupRunRecorder();

	std::vector<FileBackupRunEvent> emittedEvents;
	recorder->GetEventManager().Subscribe([&](const auto& backupRunEvent) {
		emittedEvents.push_back(backupRunEvent);
	});
	const auto bid = recorder->Start();
	emittedEvents.clear();

	// Act
	recorder->Stop(bid);

	// Assert
	ASSERT_EQ(1, emittedEvents.size());
	const auto first = *(emittedEvents.begin());
	EXPECT_EQ(FileBackupRunEventAction::Finished, first.action);
	EXPECT_EQ(bid, first.runId);
}

}
}
}
}
