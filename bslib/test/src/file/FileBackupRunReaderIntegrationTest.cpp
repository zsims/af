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
	const auto testFilePath = GetUniqueExtendedTempPath();
	const auto testFilePath2 = GetUniqueExtendedTempPath();
	WriteFile(testFilePath, "hello");
	auto fileAdder = _uow->CreateFileAdder(run1);
	fileAdder->Add(testFilePath.ToExtendedString());
	recorder->Stop(run1);
	const auto run2 = recorder->Start();
	WriteFile(testFilePath, "hell");	// Modify
	WriteFile(testFilePath2, "hello");	// add
	auto fileAdder2 = _uow->CreateFileAdder(run2);
	fileAdder2->Add(testFilePath.ToExtendedString());
	fileAdder2->Add(testFilePath2.ToExtendedString());
	recorder->Stop(run2);
	const auto run3 = recorder->Start();
	recorder->Stop(run3);
	const auto run4 = recorder->Start();
	ASSERT_EQ(7, emittedEvents.size());

	auto reader = _uow->CreateFileBackupRunReader();
	// Act
	const auto page1 = reader->Search(FileBackupRunSearchCriteria(), 0, 2);
	const auto page2 = reader->Search(FileBackupRunSearchCriteria(), 2, 2);

	// Assert
	ASSERT_EQ(2, page1.backups.size());
	EXPECT_EQ(2, page1.nextPageSkip);
	{
		const auto backup = std::find_if(page1.backups.begin(), page1.backups.end(), [&](const auto& x) { return x.runId == run4; });
		ASSERT_NE(backup, page1.backups.end());
		EXPECT_EQ(backup->startedUtc, emittedEvents[6].dateTimeUtc);
		EXPECT_FALSE(backup->finishedUtc);
		EXPECT_TRUE(backup->backupRunEvents.empty());
	}
	{
		const auto backup = std::find_if(page1.backups.begin(), page1.backups.end(), [&](const auto& x) { return x.runId == run3; });
		ASSERT_NE(backup, page1.backups.end());
		EXPECT_EQ(backup->startedUtc, emittedEvents[4].dateTimeUtc);
		EXPECT_EQ(backup->finishedUtc, emittedEvents[5].dateTimeUtc);
		EXPECT_EQ(0, backup->modifiedFilesCount);
	}

	ASSERT_EQ(2, page2.backups.size());
	EXPECT_EQ(4, page2.nextPageSkip);
	{
		const auto backup = std::find_if(page2.backups.begin(), page2.backups.end(), [&](const auto& x) { return x.runId == run2; });
		ASSERT_NE(backup, page2.backups.end());
		EXPECT_EQ(backup->startedUtc, emittedEvents[2].dateTimeUtc);
		EXPECT_EQ(backup->finishedUtc, emittedEvents[3].dateTimeUtc);
		EXPECT_EQ(2, backup->modifiedFilesCount);
		EXPECT_EQ(9, backup->totalSizeBytes);
	}
	{
		const auto backup = std::find_if(page2.backups.begin(), page2.backups.end(), [&](const auto& x) { return x.runId == run1; });
		ASSERT_NE(backup, page2.backups.end());
		EXPECT_EQ(backup->startedUtc, emittedEvents[0].dateTimeUtc);
		EXPECT_EQ(backup->finishedUtc, emittedEvents[1].dateTimeUtc);
		EXPECT_EQ(1, backup->modifiedFilesCount);
		EXPECT_EQ(5, backup->totalSizeBytes);
	}
}

TEST_F(FileBackupRunReaderIntegrationTest, Search_IncludeRunEventsSuccess)
{
	// Arrange
	auto recorder = _uow->CreateFileBackupRunRecorder();
	std::vector<FileBackupRunEvent> emittedEvents;
	recorder->GetEventManager().Subscribe([&](const auto& runEvent) {
		emittedEvents.push_back(runEvent);
	});
	const auto run1 = recorder->Start();
	recorder->Stop(run1);
	auto reader = _uow->CreateFileBackupRunReader();

	// Act
	const auto page1 = reader->Search(FileBackupRunSearchCriteria(), 0, 2, true);

	// Assert
	ASSERT_EQ(1, page1.backups.size());
	EXPECT_THAT(page1.backups[0].backupRunEvents, ::testing::UnorderedElementsAreArray(emittedEvents));
}

TEST_F(FileBackupRunReaderIntegrationTest, Search_EmptySuccess)
{
	// Arrange
	auto reader = _uow->CreateFileBackupRunReader();

	// Act
	const auto results = reader->Search(FileBackupRunSearchCriteria(), 0, 10);

	// Assert
	EXPECT_TRUE(results.backups.empty());
	EXPECT_EQ(0, results.totalBackups);
}

}
}
}
}
