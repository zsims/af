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

	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(BlobAddress("2f59225215937593795395739753973973593571"), 157UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);

	const std::vector<FileEvent> expectedEvents = {
		FileEvent("/foo", boost::none, FileEventAction::Added),
		FileEvent("/foo", blobInfo1.GetAddress(), FileEventAction::Modified),
		FileEvent("/foo", boost::none, FileEventAction::Modified),
		FileEvent("/bar", blobInfo2.GetAddress(), FileEventAction::Added),
		FileEvent("/foo", boost::none, FileEventAction::Removed)
	};
	repo.AddEvents(expectedEvents);

	// Act
	const auto& result = repo.GetAllEvents();

	// Assert
	EXPECT_EQ(result, expectedEvents);
}

TEST_F(FileEventStreamRepositoryIntegrationTest, FindLastEvent_Success)
{
	// Arrange
	FileEventStreamRepository repo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);

	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(BlobAddress("2f59225215937593795395739753973973593571"), 157UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);

	const FileEvent expectedEvent("/then that", blobInfo2.GetAddress(), FileEventAction::Added);
	const std::vector<FileEvent> events = {
		FileEvent("/something complex", boost::none, FileEventAction::Added),
		FileEvent("/then that", blobInfo1.GetAddress(), FileEventAction::Modified),
		FileEvent("/and this", boost::none, FileEventAction::Modified),
		expectedEvent,
		FileEvent("/Then that", blobInfo1.GetAddress(), FileEventAction::Modified),
		FileEvent("/something", boost::none, FileEventAction::Removed)
	};
	repo.AddEvents(events);

	// Act
	const auto& found = repo.FindLastEvent("/then that");

	// Assert
	ASSERT_TRUE(found);
	EXPECT_EQ(found.value(), expectedEvent);
}

TEST_F(FileEventStreamRepositoryIntegrationTest, FindLastEvent_CaseSensitiveSuccess)
{
	// Arrange
	FileEventStreamRepository repo(*_connection);

	const FileEvent expectedEvent(R"(C:\Foo\bar.txt)", boost::none, FileEventAction::Modified);
	const std::vector<FileEvent> events = {
		FileEvent(R"(C:\oTher root\bar.txt)", boost::none, FileEventAction::Added),
		expectedEvent,
		FileEvent(R"(C:\other root\bar.txt)", boost::none, FileEventAction::Removed),
	};
	repo.AddEvents(events);

	// Act
	const auto& found = repo.FindLastEvent(expectedEvent.fullPath);

	// Assert
	ASSERT_TRUE(found);
	EXPECT_EQ(found.value(), expectedEvent);
}

TEST_F(FileEventStreamRepositoryIntegrationTest, GetLastEventsUnderPath_Success)
{
	// Arrange
	FileEventStreamRepository repo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(BlobAddress("2f59225215937593795395739753973973593571"), 157UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);

	const std::vector<FileEvent> events = {
		FileEvent("/root folder", boost::none, FileEventAction::Added),
		FileEvent(R"(C:\other root\)", blobInfo1.GetAddress(), FileEventAction::Modified),
		FileEvent(R"(C:\other root\something\baz.txt)", blobInfo2.GetAddress(), FileEventAction::Modified),
		FileEvent(R"(C:\other something\)", boost::none, FileEventAction::Added),
		FileEvent("/root folder/home", boost::none, FileEventAction::Removed),
		FileEvent(R"(C:\other root\something\baz.txt)", blobInfo2.GetAddress(), FileEventAction::Modified),
		FileEvent(R"(C:\other root\single.txt)", blobInfo2.GetAddress(), FileEventAction::Modified),
		// Note the change in case, so shouldn't be included
		FileEvent(R"(C:\Other root\single.txt)", blobInfo2.GetAddress(), FileEventAction::Modified),
		FileEvent(R"(C:\other root\)", boost::none, FileEventAction::Modified),
		FileEvent(R"(C:\other root hehehe\)", boost::none, FileEventAction::Modified),
	};
	repo.AddEvents(events);

	const std::vector<FileEvent> expectedEvents = {
		events[5],
		events[6],
		events[8]
	};

	// Act
	const auto& result = repo.GetLastEventsStartingWithPath(R"(C:\other root\)");

	// Assert
	std::vector<FileEvent> justEvents;
	for (const auto& e : result)
	{
		justEvents.push_back(e.second);
	}
	EXPECT_THAT(justEvents, ::testing::UnorderedElementsAreArray(expectedEvents));
}

TEST_F(FileEventStreamRepositoryIntegrationTest, GetLastEventsUnderPath_HandlesSpecialCharactersSuccess)
{
	// Arrange
	FileEventStreamRepository repo(*_connection);

	const std::vector<FileEvent> events = {
		FileEvent(R"(C:\Itsa\)", boost::none, FileEventAction::Modified),
		FileEvent(R"(C:\Its_\)", boost::none, FileEventAction::Modified),
		FileEvent(R"(C:\Its_\Here)", boost::none, FileEventAction::Modified),
		FileEvent(R"(C:\Its \)", boost::none, FileEventAction::Modified),
	};
	repo.AddEvents(events);

	const std::vector<FileEvent> expectedEvents = {
		events[1],
		events[2]
	};

	// Act
	const auto& result = repo.GetLastEventsStartingWithPath(R"(C:\Its_\)");

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
		FileEvent("/look/phil/no/hands", boost::none, FileEventAction::Modified),
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
	const BlobAddress madeUpBlobAddress("2259225215937593725395732753973973593571");

	// Act
	// Assert
	ASSERT_THROW(repo.AddEvent(FileEvent("/look/phil/no/hands", madeUpBlobAddress, FileEventAction::Added)), AddFileEventFailedException);
}

}
}
}
}
