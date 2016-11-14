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

}
}
}
}
