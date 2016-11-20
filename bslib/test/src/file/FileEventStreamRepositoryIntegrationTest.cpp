#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"
#include "bslib_test_util/TestBase.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <set>

namespace af {
namespace bslib {
namespace file {
namespace test {

class FileEventStreamRepositoryIntegrationTest : public bslib_test_util::TestBase
{
protected:
	FileEventStreamRepositoryIntegrationTest()
		: _backupRunId(Uuid::Create())
	{
		_testBackup.Create();
		_connection = _testBackup.ConnectToDatabase();
	}

	const Uuid _backupRunId;
	std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
};

TEST_F(FileEventStreamRepositoryIntegrationTest, GetAllEvents_Success)
{
	// Arrange
	FileEventStreamRepository repo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);

	const blob::BlobInfo blobInfo1(blob::Address("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(blob::Address("2f59225215937593795395739753973973593571"), 157UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);

	const std::vector<FileEvent> expectedEvents = {
		FileEvent(_backupRunId, fs::NativePath("/foo"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(_backupRunId, fs::NativePath("/foo"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath("/foo"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath("/bar"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(_backupRunId, fs::NativePath("/foo"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved)
	};
	repo.AddEvents(expectedEvents);

	// Act
	const auto& result = repo.GetAllEvents();

	// Assert
	EXPECT_EQ(result, expectedEvents);
}

TEST_F(FileEventStreamRepositoryIntegrationTest, FindLastGoodEvent_Success)
{
	// Arrange
	FileEventStreamRepository repo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);

	const blob::BlobInfo blobInfo1(blob::Address("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(blob::Address("2f59225215937593795395739753973973593571"), 157UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);

	const FileEvent expectedEvent(_backupRunId, fs::NativePath("/then that"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedAdded);
	const std::vector<FileEvent> events = {
		FileEvent(_backupRunId, fs::NativePath("/something complex"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(_backupRunId, fs::NativePath("/then that"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath("/and this"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		expectedEvent,
		FileEvent(_backupRunId, fs::NativePath("/Then that"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath("/then that"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::Unchanged),
		FileEvent(_backupRunId, fs::NativePath("/something"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved)
	};
	repo.AddEvents(events);

	// Act
	const auto& found = repo.FindLastChangedEvent(fs::NativePath("/then that"));

	// Assert
	ASSERT_TRUE(found);
	EXPECT_EQ(found.value(), expectedEvent);
}

TEST_F(FileEventStreamRepositoryIntegrationTest, FindLastGoodEvent_CaseSensitiveSuccess)
{
	// Arrange
	FileEventStreamRepository repo(*_connection);

	const FileEvent expectedEvent(_backupRunId, fs::NativePath(R"(C:\Foo\bar.txt)"), FileType::RegularFile, boost::none, FileEventAction::ChangedModified);
	const std::vector<FileEvent> events = {
		FileEvent(_backupRunId, fs::NativePath(R"(C:\oTher root\bar.txt)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded),
		expectedEvent,
		FileEvent(_backupRunId, fs::NativePath(R"(C:\other root\bar.txt)"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved),
	};
	repo.AddEvents(events);

	// Act
	const auto& found = repo.FindLastChangedEvent(expectedEvent.fullPath);

	// Assert
	ASSERT_TRUE(found);
	EXPECT_EQ(found.value(), expectedEvent);
}

TEST_F(FileEventStreamRepositoryIntegrationTest, GetLastGoodEventsUnderPath_Success)
{
	// Arrange
	FileEventStreamRepository repo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(blob::Address("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(blob::Address("2f59225215937593795395739753973973593571"), 157UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);

	const std::vector<FileEvent> events = {
		FileEvent(_backupRunId, fs::NativePath("/root folder"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(_backupRunId, fs::NativePath(R"(C:\other root\)"), FileType::Directory, blobInfo1.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath(R"(C:\other root\something\baz.txt)"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath(R"(C:\other something\)"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(_backupRunId, fs::NativePath("/root folder/home"), FileType::Directory, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(_backupRunId, fs::NativePath(R"(C:\other root\something\baz.txt)"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath(R"(C:\other root\single.txt)"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedModified),
		// Note the change in case, so shouldn't be included
		FileEvent(_backupRunId, fs::NativePath(R"(C:\Other root\single.txt)"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath(R"(C:\other root\)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath(R"(C:\other root\)"), FileType::Directory, boost::none, FileEventAction::FailedToRead),
		FileEvent(_backupRunId, fs::NativePath(R"(C:\other root hehehe\)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
	};
	repo.AddEvents(events);

	const std::vector<FileEvent> expectedEvents = {
		events[5],
		events[6],
		events[8]
	};

	// Act
	const auto& result = repo.GetLastChangedEventsStartingWithPath(fs::NativePath(R"(C:\other root\)"));

	// Assert
	std::vector<FileEvent> justEvents;
	for (const auto& e : result)
	{
		justEvents.push_back(e.second);
	}
	EXPECT_THAT(justEvents, ::testing::UnorderedElementsAreArray(expectedEvents));
}

TEST_F(FileEventStreamRepositoryIntegrationTest, GetLastGoodEventsUnderPath_HandlesSpecialCharactersSuccess)
{
	// Arrange
	FileEventStreamRepository repo(*_connection);

	const std::vector<FileEvent> events = {
		FileEvent(_backupRunId, fs::NativePath(R"(C:\Itsa\)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath(R"(C:\Its_\)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath(R"(C:\Its_\Here)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath(R"(C:\Its \)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
	};
	repo.AddEvents(events);

	const std::vector<FileEvent> expectedEvents = {
		events[1],
		events[2]
	};

	// Act
	const auto& result = repo.GetLastChangedEventsStartingWithPath(fs::NativePath(R"(C:\Its_\)"));

	// Assert
	std::vector<FileEvent> justEvents;
	for (const auto& e : result)
	{
		justEvents.push_back(e.second);
	}
	EXPECT_THAT(justEvents, ::testing::UnorderedElementsAreArray(expectedEvents));
}

TEST_F(FileEventStreamRepositoryIntegrationTest, AddEvent_NoBlobSuccess)
{
	// Arrange
	FileEventStreamRepository repo(*_connection);

	const std::vector<FileEvent> events = {
		FileEvent(_backupRunId, fs::NativePath("/look/phil/no/hands"), FileType::RegularFile, boost::none, FileEventAction::ChangedModified),
	};

	// Act
	repo.AddEvent(events[0]);

	// Assert
	const auto& result = repo.GetAllEvents();
	EXPECT_EQ(result, events);
}

TEST_F(FileEventStreamRepositoryIntegrationTest, AddEvent_MissingBlobThrows)
{
	// Arrange
	FileEventStreamRepository repo(*_connection);
	const blob::Address madeUpBlobAddress("2259225215937593725395732753973973593571");

	// Act
	// Assert
	ASSERT_THROW(repo.AddEvent(FileEvent(_backupRunId, fs::NativePath("/look/phil/no/hands"), FileType::RegularFile, madeUpBlobAddress, FileEventAction::ChangedAdded)), AddFileEventFailedException);
}


TEST_F(FileEventStreamRepositoryIntegrationTest, GetStatisticsByRunId_Success)
{
	// Arrange
	FileEventStreamRepository repo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(blob::Address("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(blob::Address("2f59225215937593795395739753973973593571"), 157UL);
	const blob::BlobInfo blobInfo3(blob::Address("4e59225215937593795395739753973973593571"), 1337UL);
	const blob::BlobInfo blobInfo4(blob::Address("5e59225215937593795395739753973973593571"), 1UL);
	const blob::BlobInfo blobInfo5(blob::Address("6959225215937593795395739753973973593571"), 69UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);
	blobRepo.AddBlob(blobInfo3);
	blobRepo.AddBlob(blobInfo4);
	blobRepo.AddBlob(blobInfo5);

	const auto run1 = Uuid::Create();
	const auto run2 = Uuid::Create();
	const auto run3 = Uuid::Create();

	const std::vector<FileEvent> expectedEvents = {
		FileEvent(run1, fs::NativePath("/dir"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath("/file"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath("/otherfile"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath("/file"), FileType::RegularFile, blobInfo3.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(run1, fs::NativePath("/old"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(run2, fs::NativePath("/file"), FileType::Directory, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(run3, fs::NativePath("/file"), FileType::RegularFile, blobInfo4.GetAddress(), FileEventAction::ChangedRemoved),
		FileEvent(run3, fs::NativePath("/69"), FileType::RegularFile, blobInfo5.GetAddress(), FileEventAction::ChangedAdded)
	};
	repo.AddEvents(expectedEvents);

	// Act
	const auto stats = repo.GetStatisticsByRunId(
		std::vector<Uuid>{run1, run2, run3},
		std::set<FileEventAction>{FileEventAction::ChangedAdded, FileEventAction::ChangedModified});

	// Assert
	{
		const auto runStats = stats.find(run1);
		EXPECT_EQ(444UL + 157UL + 1337UL, runStats->second.matchingSizeBytes);
		EXPECT_EQ(4, runStats->second.matchingEvents);
	}
	{
		const auto runStats = stats.find(run2);
		EXPECT_EQ(0, runStats->second.matchingSizeBytes);
		EXPECT_EQ(0, runStats->second.matchingEvents);
	}
	{
		const auto runStats = stats.find(run3);
		EXPECT_EQ(69, runStats->second.matchingSizeBytes);
		EXPECT_EQ(1, runStats->second.matchingEvents);
	}
}

}
}
}
}
