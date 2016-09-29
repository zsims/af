#include "bslib/forest.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace file {
namespace test {

class FileEventStreamRepositoryIntegrationTest : public testing::Test
{
protected:
	FileEventStreamRepositoryIntegrationTest()
		: _forestDbPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.fdb"))
	{
		Forest forest(_forestDbPath.string(), std::make_unique<blob::NullBlobStore>());
		forest.Create();

		_connection = std::make_unique<sqlitepp::ScopedSqlite3Object>();
		sqlitepp::open_database_or_throw(_forestDbPath.string().c_str(), *_connection, SQLITE_OPEN_READWRITE);
		sqlitepp::exec_or_throw(*_connection, "PRAGMA case_sensitive_like = true;");
	}

	~FileEventStreamRepositoryIntegrationTest()
	{
		_connection.reset();
		boost::system::error_code ec;
		boost::filesystem::remove(_forestDbPath, ec);
	}

	std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
	const boost::filesystem::path _forestDbPath;
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
		FileEvent(fs::NativePath("/foo"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(fs::NativePath("/foo"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(fs::NativePath("/foo"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		FileEvent(fs::NativePath("/bar"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedAdded),
		FileEvent(fs::NativePath("/foo"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved)
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

	const FileEvent expectedEvent(fs::NativePath("/then that"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedAdded);
	const std::vector<FileEvent> events = {
		FileEvent(fs::NativePath("/something complex"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(fs::NativePath("/then that"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(fs::NativePath("/and this"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		expectedEvent,
		FileEvent(fs::NativePath("/Then that"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(fs::NativePath("/then that"), FileType::RegularFile, blobInfo1.GetAddress(), FileEventAction::Unchanged),
		FileEvent(fs::NativePath("/something"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved)
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

	const FileEvent expectedEvent(fs::NativePath(R"(C:\Foo\bar.txt)"), FileType::RegularFile, boost::none, FileEventAction::ChangedModified);
	const std::vector<FileEvent> events = {
		FileEvent(fs::NativePath(R"(C:\oTher root\bar.txt)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded),
		expectedEvent,
		FileEvent(fs::NativePath(R"(C:\other root\bar.txt)"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved),
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
		FileEvent(fs::NativePath("/root folder"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(fs::NativePath(R"(C:\other root\)"), FileType::Directory, blobInfo1.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(fs::NativePath(R"(C:\other root\something\baz.txt)"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(fs::NativePath(R"(C:\other something\)"), FileType::Directory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(fs::NativePath("/root folder/home"), FileType::Directory, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(fs::NativePath(R"(C:\other root\something\baz.txt)"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(fs::NativePath(R"(C:\other root\single.txt)"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedModified),
		// Note the change in case, so shouldn't be included
		FileEvent(fs::NativePath(R"(C:\Other root\single.txt)"), FileType::RegularFile, blobInfo2.GetAddress(), FileEventAction::ChangedModified),
		FileEvent(fs::NativePath(R"(C:\other root\)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		FileEvent(fs::NativePath(R"(C:\other root\)"), FileType::Directory, boost::none, FileEventAction::FailedToRead),
		FileEvent(fs::NativePath(R"(C:\other root hehehe\)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
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
		FileEvent(fs::NativePath(R"(C:\Itsa\)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		FileEvent(fs::NativePath(R"(C:\Its_\)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		FileEvent(fs::NativePath(R"(C:\Its_\Here)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
		FileEvent(fs::NativePath(R"(C:\Its \)"), FileType::Directory, boost::none, FileEventAction::ChangedModified),
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
		FileEvent(fs::NativePath("/look/phil/no/hands"), FileType::RegularFile, boost::none, FileEventAction::ChangedModified),
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
	ASSERT_THROW(repo.AddEvent(FileEvent(fs::NativePath("/look/phil/no/hands"), FileType::RegularFile, madeUpBlobAddress, FileEventAction::ChangedAdded)), AddFileEventFailedException);
}

}
}
}
}
