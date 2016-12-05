#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"
#include "bslib/file/FilePathRepository.hpp"
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
		_fileEventStreamRepository = std::make_unique<FileEventStreamRepository>(*_connection);
		_filePathRepository = std::make_unique<FilePathRepository>(*_connection);
	}

	int64_t GetPathId(const fs::NativePath& path)
	{
		const auto existingPathId = _filePathRepository->FindPath(path);
		if (existingPathId)
		{
			return existingPathId.value();
		}
		return _filePathRepository->AddPath(path);
	}

	void AddEvent(const FileEvent& fileEvent)
	{
		auto pathId = GetPathId(fileEvent.fullPath);
		const auto pathSegments = fileEvent.fullPath.GetIntermediatePaths();
		unsigned distance = static_cast<unsigned>(pathSegments.size());
		for (const auto& segment : pathSegments)
		{
			_filePathRepository->AddParent(pathId, GetPathId(segment), --distance);
		}

		_fileEventStreamRepository->AddEvent(fileEvent, pathId);
	}

	void AddEvents(const std::vector<FileEvent>& events)
	{
		for (const auto& e : events)
		{
			AddEvent(e);
		}
	}

	const Uuid _backupRunId;
	std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
	std::unique_ptr<FileEventStreamRepository> _fileEventStreamRepository;
	std::unique_ptr<FilePathRepository> _filePathRepository;
};

TEST_F(FileEventStreamRepositoryIntegrationTest, GetAllEvents_Success)
{
	// Arrange
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
	AddEvents(expectedEvents);

	// Act
	const auto& result = _fileEventStreamRepository->GetAllEvents();

	// Assert
	EXPECT_EQ(result, expectedEvents);
}

TEST_F(FileEventStreamRepositoryIntegrationTest, FindLastGoodEvent_Success)
{
	// Arrange
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
	AddEvents(events);

	// Act
	const auto& found = _fileEventStreamRepository->FindLastChangedEvent(fs::NativePath("/then that"));

	// Assert
	ASSERT_TRUE(found);
	EXPECT_EQ(found.value(), expectedEvent);
}

TEST_F(FileEventStreamRepositoryIntegrationTest, FindLastGoodEvent_CaseSensitiveSuccess)
{
	// Arrange
	const FileEvent expectedEvent(_backupRunId, fs::NativePath(R"(C:\Foo\bar.txt)"), FileType::RegularFile, boost::none, FileEventAction::ChangedModified);
	const std::vector<FileEvent> events = {
		FileEvent(_backupRunId, fs::NativePath(R"(C:\oTher root\bar.txt)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded),
		expectedEvent,
		FileEvent(_backupRunId, fs::NativePath(R"(C:\other root\bar.txt)"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved),
	};
	AddEvents(events);

	// Act
	const auto& found = _fileEventStreamRepository->FindLastChangedEvent(expectedEvent.fullPath);

	// Assert
	ASSERT_TRUE(found);
	EXPECT_EQ(found.value(), expectedEvent);
}

TEST_F(FileEventStreamRepositoryIntegrationTest, GetLastGoodEventsUnderPath_Success)
{
	// Arrange
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
	AddEvents(events);

	const std::vector<FileEvent> expectedEvents = {
		events[5],
		events[6],
		events[8]
	};

	// Act
	const auto& result = _fileEventStreamRepository->GetLastChangedEventsUnderPath(fs::NativePath(R"(C:\other root\)"));

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
	const std::vector<FileEvent> events = {
		FileEvent(_backupRunId, fs::NativePath(R"(C:\Itsa\)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath(R"(C:\Its_\)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath(R"(C:\Its_\Here)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		FileEvent(_backupRunId, fs::NativePath(R"(C:\Its \)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
	};
	AddEvents(events);

	const std::vector<FileEvent> expectedEvents = {
		events[1],
		events[2]
	};

	// Act
	const auto& result = _fileEventStreamRepository->GetLastChangedEventsUnderPath(fs::NativePath(R"(C:\Its_\)"));

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
	const std::vector<FileEvent> events = {
		FileEvent(_backupRunId, fs::NativePath("/look/phil/no/hands"), FileType::RegularFile, boost::none, FileEventAction::ChangedModified),
	};

	// Act
	AddEvent(events[0]);

	// Assert
	const auto& result = _fileEventStreamRepository->GetAllEvents();
	EXPECT_EQ(result, events);
}

TEST_F(FileEventStreamRepositoryIntegrationTest, AddEvent_MissingBlobThrows)
{
	// Arrange
	const blob::Address madeUpBlobAddress("2259225215937593725395732753973973593571");

	// Act
	// Assert
	ASSERT_THROW(AddEvent(FileEvent(_backupRunId, fs::NativePath("/look/phil/no/hands"), FileType::RegularFile, madeUpBlobAddress, FileEventAction::ChangedAdded)), AddFileEventFailedException);
}

TEST_F(FileEventStreamRepositoryIntegrationTest, AddEvent_MissingPathThrows)
{
	// Arrange
	// Act
	// Assert
	ASSERT_THROW(_fileEventStreamRepository->AddEvent(FileEvent(_backupRunId, fs::NativePath("/look/phil/no/hands"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded), 69), AddFileEventFailedException);
}

TEST_F(FileEventStreamRepositoryIntegrationTest, GetStatisticsByRunId_Success)
{
	// Arrange
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
	AddEvents(expectedEvents);

	// Act
	const auto stats = _fileEventStreamRepository->GetStatisticsByRunId(
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

TEST_F(FileEventStreamRepositoryIntegrationTest, Search_Success)
{
	// Arrange
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(blob::Address("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(blob::Address("2f59225215937593795395739753973973593571"), 157UL);
	const blob::BlobInfo blobInfo3(blob::Address("4e59225215937593795395739753973973593571"), 1337UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);
	blobRepo.AddBlob(blobInfo3);

	const auto run1 = Uuid::Create();
	const auto run2 = Uuid::Create();

	const std::vector<FileEvent> expectedEvents = {
		FileEvent(run1, fs::NativePath("/dir"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath("/file"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath("/otherfile"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath("/file"), FileType::RegularFile, blobInfo3.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(run1, fs::NativePath("/old"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(run2, fs::NativePath("/file"), FileType::Directory, boost::none, FileEventAction::ChangedRemoved),
	};
	AddEvents(expectedEvents);

	// Act
	FileEventSearchCriteria criteria;
	const auto page1 = _fileEventStreamRepository->Search(criteria, 0, 4);
	const auto page2 = _fileEventStreamRepository->Search(criteria, 4, 4);

	// Assert
	EXPECT_THAT(page1, ::testing::ElementsAre(expectedEvents[0], expectedEvents[1], expectedEvents[2], expectedEvents[3]));
	EXPECT_THAT(page2, ::testing::ElementsAre(expectedEvents[4], expectedEvents[5]));
}

TEST_F(FileEventStreamRepositoryIntegrationTest, Search_ByActionSuccess)
{
	// Arrange
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(blob::Address("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(blob::Address("2f59225215937593795395739753973973593571"), 157UL);
	const blob::BlobInfo blobInfo3(blob::Address("4e59225215937593795395739753973973593571"), 1337UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);
	blobRepo.AddBlob(blobInfo3);

	const auto run1 = Uuid::Create();
	const auto run2 = Uuid::Create();

	const std::vector<FileEvent> expectedEvents = {
		FileEvent(run1, fs::NativePath(R"(C:\dir)"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(C:\file)"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(C:\otherfile)"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(C:\file)"), FileType::RegularFile, blobInfo3.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(run1, fs::NativePath(R"(C:\old)"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(run2, fs::NativePath(R"(C:\file)"), FileType::Directory, boost::none, FileEventAction::ChangedRemoved),
	};
	AddEvents(expectedEvents);

	// Act
	FileEventSearchCriteria criteria;
	criteria.actions = std::set<FileEventAction>{ FileEventAction::ChangedAdded, FileEventAction::ChangedModified };
	const auto page1 = _fileEventStreamRepository->Search(criteria, 0, 2);
	const auto page2 = _fileEventStreamRepository->Search(criteria, 2, 2);

	const auto matching = _fileEventStreamRepository->CountMatching(criteria);
	EXPECT_THAT(4, matching);

	// Assert
	EXPECT_THAT(page1, ::testing::ElementsAre(expectedEvents[0], expectedEvents[1]));
	EXPECT_THAT(page2, ::testing::ElementsAre(expectedEvents[2], expectedEvents[3]));
}

TEST_F(FileEventStreamRepositoryIntegrationTest, Search_ByRunIdSuccess)
{
	// Arrange
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(blob::Address("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(blob::Address("2f59225215937593795395739753973973593571"), 157UL);
	const blob::BlobInfo blobInfo3(blob::Address("4e59225215937593795395739753973973593571"), 1337UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);
	blobRepo.AddBlob(blobInfo3);

	const auto run1 = Uuid::Create();
	const auto run2 = Uuid::Create();

	const std::vector<FileEvent> expectedEvents = {
		FileEvent(run1, fs::NativePath(R"(C:\dir)"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(C:\file)"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(C:\otherfile)"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(C:\file)"), FileType::RegularFile, blobInfo3.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(run1, fs::NativePath(R"(C:\old)"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(run2, fs::NativePath(R"(C:\file)"), FileType::Directory, boost::none, FileEventAction::ChangedRemoved),
	};
	AddEvents(expectedEvents);

	// Act
	FileEventSearchCriteria criteria;
	criteria.actions = std::set<FileEventAction>{ FileEventAction::ChangedRemoved};
	criteria.runId = run1;
	const auto page1 = _fileEventStreamRepository->Search(criteria, 0, 4);

	const auto matching = _fileEventStreamRepository->CountMatching(criteria);
	EXPECT_THAT(1, matching);

	// Assert
	EXPECT_THAT(page1, ::testing::ElementsAre(expectedEvents[4]));
}

TEST_F(FileEventStreamRepositoryIntegrationTest, Search_ByAncestorPathIdSuccess)
{
	// Arrange
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(blob::Address("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(blob::Address("2f59225215937593795395739753973973593571"), 157UL);
	const blob::BlobInfo blobInfo3(blob::Address("4e59225215937593795395739753973973593571"), 1337UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);
	blobRepo.AddBlob(blobInfo3);

	const auto run1 = Uuid::Create();
	const auto run2 = Uuid::Create();

	const fs::NativePath rootPath(R"(A:\)");
	const std::vector<FileEvent> expectedEvents = {
		FileEvent(run1, rootPath, FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(A:\file)"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(B:\otherfile)"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(B:\file)"), FileType::RegularFile, blobInfo3.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(run1, fs::NativePath(R"(C:\old)"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(run2, fs::NativePath(R"(C:\file)"), FileType::Directory, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(run1, fs::NativePath(R"(A:\file\that\is\deep)"), FileType::RegularFile, boost::none , FileEventAction::ChangedAdded),
	};
	AddEvents(expectedEvents);

	const auto rootPathId = _filePathRepository->FindPath(rootPath);
	ASSERT_TRUE(rootPathId);

	// Act
	FileEventSearchCriteria criteria;
	criteria.actions = { FileEventAction::ChangedRemoved, FileEventAction::ChangedAdded };
	criteria.runId = run1;
	criteria.ancestorPathId = rootPathId;
	const auto page1 = _fileEventStreamRepository->Search(criteria, 0, 4);

	const auto matching = _fileEventStreamRepository->CountMatching(criteria);
	EXPECT_THAT(3, matching);

	// Assert
	EXPECT_THAT(page1, ::testing::ElementsAre(expectedEvents[0], expectedEvents[1], expectedEvents[6]));
}

TEST_F(FileEventStreamRepositoryIntegrationTest, SearchDistinctPath_ByAncestorPathIdSuccess)
{
	// Arrange
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(blob::Address("1259225215937593795395739753973973593571"), 444UL);
	blobRepo.AddBlob(blobInfo1);

	const auto run1 = Uuid::Create();
	const auto run2 = Uuid::Create();

	const fs::NativePath rootPath(R"(A:\)");
	const std::vector<FileEvent> expectedEvents = {
		FileEvent(run1, rootPath, FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(A:\file)"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(run2, fs::NativePath(R"(C:\file)"), FileType::Directory, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(run1, fs::NativePath(R"(A:\file)"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(run1, fs::NativePath(R"(A:\extra)"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedRemoved),
		FileEvent(run1, fs::NativePath(R"(A:\file\that\is\deep)"), FileType::RegularFile, boost::none , FileEventAction::ChangedAdded),
	};
	AddEvents(expectedEvents);

	const auto rootPathId = _filePathRepository->FindPath(rootPath);
	ASSERT_TRUE(rootPathId);

	// Act
	FileEventSearchCriteria criteria;
	criteria.actions = std::set<FileEventAction>{ FileEventAction::ChangedRemoved, FileEventAction::ChangedModified, FileEventAction::ChangedAdded};
	criteria.ancestorPathId = rootPathId;
	const auto page1 = _fileEventStreamRepository->SearchDistinctPath(criteria, 0, 4);

	const auto matching = _fileEventStreamRepository->CountMatchingDistinctPath(criteria);
	EXPECT_EQ(4, matching);

	// Assert
	EXPECT_EQ(4, page1.size());
	EXPECT_THAT(page1, ::testing::UnorderedElementsAre(expectedEvents[0], expectedEvents[3], expectedEvents[4], expectedEvents[5]));
}

TEST_F(FileEventStreamRepositoryIntegrationTest, Search_ByAncestorPathIdWithDistanceSuccess)
{
	// Arrange
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo2(blob::Address("2f59225215937593795395739753973973593571"), 157UL);
	const blob::BlobInfo blobInfo3(blob::Address("4e59225215937593795395739753973973593571"), 1337UL);
	blobRepo.AddBlob(blobInfo2);
	blobRepo.AddBlob(blobInfo3);

	const auto run1 = Uuid::Create();
	const auto run2 = Uuid::Create();

	const fs::NativePath folderPath(R"(A:\folder\)");
	const std::vector<FileEvent> expectedEvents = {
		FileEvent(run1, fs::NativePath(R"(A:\)"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, folderPath, FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(C:\old)"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(run2, fs::NativePath(R"(C:\folder)"), FileType::Directory, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(run1, fs::NativePath(R"(A:\folder\that)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(A:\folder\other)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(A:\folder\that\is)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded),
	};
	AddEvents(expectedEvents);

	const auto folderPathId = _filePathRepository->FindPath(folderPath);
	ASSERT_TRUE(folderPathId);

	// Act
	FileEventSearchCriteria criteria;
	criteria.actions = { FileEventAction::ChangedRemoved, FileEventAction::ChangedAdded };
	criteria.runId = run1;
	criteria.ancestorPathId = folderPathId;
	criteria.ancestorPathDistance = 1;
	const auto page1 = _fileEventStreamRepository->Search(criteria, 0, 4);

	const auto matching = _fileEventStreamRepository->CountMatching(criteria);
	EXPECT_THAT(2, matching);

	// Assert
	EXPECT_THAT(page1, ::testing::ElementsAre(expectedEvents[4], expectedEvents[5]));
}

TEST_F(FileEventStreamRepositoryIntegrationTest, SearchDistinctPath_ByAncestorPathIdWithDistanceSuccess)
{
	// Arrange
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(blob::Address("1259225215937593795395739753973973593571"), 444UL);
	blobRepo.AddBlob(blobInfo1);

	const auto run1 = Uuid::Create();
	const auto run2 = Uuid::Create();

	const fs::NativePath folderPath(R"(A:\folder\)");
	const std::vector<FileEvent> expectedEvents = {
		FileEvent(run1, fs::NativePath(R"(A:\)"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, folderPath, FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(C:\old)"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(run2, fs::NativePath(R"(C:\folder)"), FileType::Directory, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(run1, fs::NativePath(R"(A:\folder\that)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(A:\folder\other)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run2, fs::NativePath(R"(A:\folder\other)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(A:\folder\that\is)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded),
	};
	AddEvents(expectedEvents);

	const auto folderPathId = _filePathRepository->FindPath(folderPath);
	ASSERT_TRUE(folderPathId);

	// Act
	FileEventSearchCriteria criteria;
	criteria.actions = std::set<FileEventAction>{ FileEventAction::ChangedRemoved, FileEventAction::ChangedModified, FileEventAction::ChangedAdded};
	criteria.ancestorPathId = folderPathId;
	criteria.ancestorPathDistance = 1;
	const auto page1 = _fileEventStreamRepository->SearchDistinctPath(criteria, 0, 4);

	const auto matching = _fileEventStreamRepository->CountMatchingDistinctPath(criteria);
	EXPECT_EQ(2, matching);

	// Assert
	EXPECT_EQ(2, page1.size());
	EXPECT_THAT(page1, ::testing::UnorderedElementsAre(expectedEvents[4], expectedEvents[6]));
}

TEST_F(FileEventStreamRepositoryIntegrationTest, SearchDistinctPath_ByPathDepthSuccess)
{
	// Arrange
	const auto run1 = Uuid::Create();

	const std::vector<FileEvent> expectedEvents = {
		FileEvent(run1, fs::NativePath(R"(A:\)"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(B:\)"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(A:\)"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(C:\old)"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(run1, fs::NativePath(R"(D:\old)"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved),
	};
	AddEvents(expectedEvents);

	// Act
	FileEventSearchCriteria criteria;
	criteria.actions = { FileEventAction::ChangedRemoved, FileEventAction::ChangedModified, FileEventAction::ChangedAdded };
	criteria.pathDepth = 0;
	const auto page1 = _fileEventStreamRepository->SearchDistinctPath(criteria, 0, 4);

	const auto matching = _fileEventStreamRepository->CountMatchingDistinctPath(criteria);
	EXPECT_EQ(2, matching);

	// Assert
	EXPECT_EQ(2, page1.size());
	EXPECT_THAT(page1, ::testing::UnorderedElementsAre(expectedEvents[1], expectedEvents[2]));
}

TEST_F(FileEventStreamRepositoryIntegrationTest, SearchDistinctPath_ByAncestorPathIdWithReducedActionsSuccess)
{
	// Arrange
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(blob::Address("1259225215937593795395739753973973593571"), 444UL);
	blobRepo.AddBlob(blobInfo1);

	const auto run1 = Uuid::Create();
	const auto run2 = Uuid::Create();

	const fs::NativePath rootPath(R"(A:\)");
	const std::vector<FileEvent> expectedEvents = {
		FileEvent(run1, rootPath, FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(A:\file)"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(run2, fs::NativePath(R"(C:\file)"), FileType::Directory, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(run1, fs::NativePath(R"(A:\file)"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(run1, fs::NativePath(R"(A:\extra)"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedRemoved)
	};
	AddEvents(expectedEvents);

	const auto rootPathId = _filePathRepository->FindPath(rootPath);
	ASSERT_TRUE(rootPathId);

	// Act
	FileEventSearchCriteria criteria;
	criteria.actions = std::set<FileEventAction>{ FileEventAction::ChangedRemoved, FileEventAction::ChangedAdded };
	criteria.ancestorPathId = rootPathId;
	const auto page1 = _fileEventStreamRepository->SearchDistinctPath(criteria, std::set<FileEventAction> { FileEventAction::ChangedAdded }, 0, 4);

	const auto matching = _fileEventStreamRepository->CountMatchingDistinctPath(criteria);
	// note the count is against the criteria only :). CUT YO'SELF
	EXPECT_EQ(3, matching);

	// Assert
	EXPECT_EQ(2, page1.size());
	EXPECT_THAT(page1, ::testing::UnorderedElementsAre(expectedEvents[0], expectedEvents[3]));
}

}
}
}
}
