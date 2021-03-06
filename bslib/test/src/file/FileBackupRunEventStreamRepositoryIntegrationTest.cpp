#include "bslib/file/FileBackupRunEventStreamRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"
#include "bslib_test_util/TestBase.hpp"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace file {
namespace test {

class FileBackupRunEventStreamRepositoryIntegrationTest : public bslib_test_util::TestBase
{
protected:
	FileBackupRunEventStreamRepositoryIntegrationTest()
	{
		_testBackup.Create();
		_connection = _testBackup.ConnectToDatabase();
	}

	std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
};

TEST_F(FileBackupRunEventStreamRepositoryIntegrationTest, GetAllEvents_Success)
{
	// Arrange
	FileBackupRunEventStreamRepository repo(*_connection);

	const std::vector<FileBackupRunEvent> events = {
		FileBackupRunEvent(Uuid::Empty, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Started),
		FileBackupRunEvent(Uuid::Empty, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Finished),
		FileBackupRunEvent(Uuid::Empty, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Started),
		FileBackupRunEvent(Uuid::Empty, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Started),
		FileBackupRunEvent(Uuid::Empty, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Finished),
	};

	// Act
	repo.AddEvents(events);

	// Act
	const auto& result = repo.GetAllEvents();

	// Assert
	EXPECT_THAT(result, ::testing::ElementsAreArray(events));
}

TEST_F(FileBackupRunEventStreamRepositoryIntegrationTest, SearchByRun_Success)
{
	// Arrange
	FileBackupRunEventStreamRepository repo(*_connection);

	const auto run1 = Uuid::Create();
	const auto run2 = Uuid::Create();
	const auto run3 = Uuid::Create();

	const FileBackupRunEvent e1(run1, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Started);
	const FileBackupRunEvent e2(run1, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Finished);
	const FileBackupRunEvent e3(run2, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Started);
	const FileBackupRunEvent e4(run3, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Started);
	const FileBackupRunEvent e5(run3, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Finished);
	repo.AddEvent(e1);
	repo.AddEvent(e2);
	repo.AddEvent(e3);
	repo.AddEvent(e4);
	repo.AddEvent(e5);

	// Act
	const auto page1 = repo.SearchByRun(FileBackupRunSearchCriteria(), 0, 2);
	const auto page2 = repo.SearchByRun(FileBackupRunSearchCriteria(), 2, 2);

	// Assert
	EXPECT_THAT(page1, ::testing::ElementsAre(e5, e4, e3));
	EXPECT_THAT(page2, ::testing::ElementsAre(e2, e1));
}

TEST_F(FileBackupRunEventStreamRepositoryIntegrationTest, SearchByRun_SpecificRunSuccess)
{
	// Arrange
	FileBackupRunEventStreamRepository repo(*_connection);

	const auto run1 = Uuid::Create();
	const auto run2 = Uuid::Create();

	const FileBackupRunEvent e1(run1, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Started);
	const FileBackupRunEvent e2(run1, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Finished);
	const FileBackupRunEvent e3(run2, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Started);
	repo.AddEvent(e1);
	repo.AddEvent(e2);
	repo.AddEvent(e3);

	// Act
	FileBackupRunSearchCriteria criteria;
	criteria.runId = run1;
	const auto page = repo.SearchByRun(criteria, 0, 1);

	// Assert
	EXPECT_THAT(page, ::testing::ElementsAre(e2, e1));
}

TEST_F(FileBackupRunEventStreamRepositoryIntegrationTest, GetBackupCount_Success)
{
	// Arrange
	FileBackupRunEventStreamRepository repo(*_connection);

	const auto run1 = Uuid::Create();
	const auto run2 = Uuid::Create();
	const auto run3 = Uuid::Create();

	const FileBackupRunEvent e1(run1, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Started);
	const FileBackupRunEvent e2(run1, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Finished);
	const FileBackupRunEvent e3(run2, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Started);
	const FileBackupRunEvent e4(run3, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Started);
	const FileBackupRunEvent e5(run3, boost::posix_time::second_clock::universal_time(), FileBackupRunEventAction::Finished);
	repo.AddEvent(e1);
	repo.AddEvent(e2);
	repo.AddEvent(e3);
	repo.AddEvent(e4);
	repo.AddEvent(e5);

	// Act
	const auto result = repo.GetBackupCount();

	// Assert
	EXPECT_EQ(3, result);
}

}
}
}
}
