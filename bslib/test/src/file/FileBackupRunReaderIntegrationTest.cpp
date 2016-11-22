#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileBackupRunReader.hpp"
#include "bslib_test_util/TestBase.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <algorithm>
#include <functional>
#include <memory>

namespace af {
namespace bslib {
namespace file {
namespace test {

class FileBackupRunReaderIntegrationTest : public bslib_test_util::TestBase
{
protected:
	FileBackupRunReaderIntegrationTest()
	{
		_testBackup.Create();
		_uow = _testBackup.GetBackup().CreateUnitOfWork();
	}

	std::unique_ptr<UnitOfWork> _uow;
};

TEST_F(FileBackupRunReaderIntegrationTest, Search_Success)
{
	// Arrange
	auto recorder = _uow->CreateFileBackupRunRecorder();
	std::vector<FileBackupRunEvent> emittedEvents;
	recorder->GetEventManager().Subscribe([&](const auto& runEvent) {
		emittedEvents.push_back(runEvent);
	});
	const auto run1 = recorder->Start();
	recorder->Stop(run1);
	const auto run2 = recorder->Start();
	recorder->Stop(run2);
	const auto run3 = recorder->Start();
	recorder->Stop(run3);
	const auto run4 = recorder->Start();
	ASSERT_EQ(7, emittedEvents.size());

	auto reader = _uow->CreateFileBackupRunReader();
	// Act
	const auto page1 = reader->Search(FileBackupRunSearchCriteria(0, 2));
	const auto page2 = reader->Search(FileBackupRunSearchCriteria(2, 2));

	// Assert
	ASSERT_EQ(2, page1.backups.size());
	EXPECT_EQ(2, page1.nextPageSkip);
	{
		const auto backup = std::find_if(page1.backups.begin(), page1.backups.end(), [&](const auto& x) { return x.runId == run4; });
		ASSERT_NE(backup, page1.backups.end());
		EXPECT_EQ(backup->startedUtc, emittedEvents[6].dateTimeUtc);
		EXPECT_FALSE(backup->finishedUtc);
	}
	{
		const auto backup = std::find_if(page1.backups.begin(), page1.backups.end(), [&](const auto& x) { return x.runId == run3; });
		ASSERT_NE(backup, page1.backups.end());
		EXPECT_EQ(backup->startedUtc, emittedEvents[4].dateTimeUtc);
		EXPECT_EQ(backup->finishedUtc, emittedEvents[5].dateTimeUtc);
	}

	ASSERT_EQ(2, page2.backups.size());
	EXPECT_EQ(4, page2.nextPageSkip);
	{
		const auto backup = std::find_if(page2.backups.begin(), page2.backups.end(), [&](const auto& x) { return x.runId == run2; });
		ASSERT_NE(backup, page2.backups.end());
		EXPECT_EQ(backup->startedUtc, emittedEvents[2].dateTimeUtc);
		EXPECT_EQ(backup->finishedUtc, emittedEvents[3].dateTimeUtc);
	}
	{
		const auto backup = std::find_if(page2.backups.begin(), page2.backups.end(), [&](const auto& x) { return x.runId == run1; });
		ASSERT_NE(backup, page2.backups.end());
		EXPECT_EQ(backup->startedUtc, emittedEvents[0].dateTimeUtc);
		EXPECT_EQ(backup->finishedUtc, emittedEvents[1].dateTimeUtc);
	}
}

TEST_F(FileBackupRunReaderIntegrationTest, Search_EmptySuccess)
{
	// Arrange
	auto reader = _uow->CreateFileBackupRunReader();

	// Act
	const auto results = reader->Search(FileBackupRunSearchCriteria(0, 10));

	// Assert
	EXPECT_TRUE(results.backups.empty());
	EXPECT_EQ(0, results.totalBackups);
}

}
}
}
}
