#include "bslib/file/exceptions.hpp"
#include "bslib/file/fs/operations.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"
#include "file/test_utility/ScopedWorkingDirectory.hpp"
#include "test_util/TestBase.hpp"
#include "test_util/gtest_boost_filesystem_fix.hpp"
#include "test_util/matchers.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace file {
namespace test {

class FileRestorerIntegrationTest : public test_util::TestBase
{
protected:
	FileRestorerIntegrationTest()
		: _restorePath(GetUniqueExtendedTempPath())
		, _sampleBasePath(GetUniqueExtendedTempPath())
		, _sampleSubDirectory(_sampleBasePath / "sub")
		, _sampleFilePath(_sampleBasePath / "base.dat")
		, _sampleSubFilePath(_sampleSubDirectory / "subfile.dat")
	{
		_testBackup.Create();
		fs::CreateDirectories(_restorePath);

		_uow = _testBackup.GetBackup().CreateUnitOfWork();
		_adder = _uow->CreateFileAdder();
		_restorer = _uow->CreateFileRestorer();
		_finder = _uow->CreateFileFinder();

		// Test data
		fs::CreateDirectories(_sampleBasePath);
		fs::CreateDirectories(_sampleSubDirectory);
		CreateFile(_sampleFilePath, "hey babe");
		CreateFile(_sampleSubFilePath, "hey sub babe");
	}

	const fs::NativePath _restorePath;
	std::unique_ptr<UnitOfWork> _uow;
	std::unique_ptr<FileAdder> _adder;
	std::unique_ptr<FileRestorer> _restorer;
	std::unique_ptr<FileFinder> _finder;

	const fs::NativePath _sampleBasePath;
	const fs::NativePath _sampleSubDirectory;
	const fs::NativePath _sampleFilePath;
	const fs::NativePath _sampleSubFilePath;
};

TEST_F(FileRestorerIntegrationTest, Restore_ThrowsIfTargetDoNotExist)
{
	// Arrange
	const auto restorePath = GetUniqueExtendedTempPath();
	// Act
	// Assert
	ASSERT_THROW(_restorer->Restore(std::vector<FileEvent>(), restorePath.ToString()), TargetPathNotSupportedException);
}

TEST_F(FileRestorerIntegrationTest, Restore_ThrowsIfTargetIsFile)
{
	// Arrange
	const auto restorePath = GetUniqueExtendedTempPath();
	CreateFile(restorePath, "hello");
	// Act
	// Assert
	ASSERT_THROW(_restorer->Restore(std::vector<FileEvent>(), restorePath.ToString()), TargetPathNotSupportedException);
}

TEST_F(FileRestorerIntegrationTest, Restore_FileToFilePath)
{
	// Arrange
	const auto fileRestorePath = _restorePath / _sampleFilePath.GetFilename();
	_adder->Add(_sampleFilePath.ToString());

	std::vector<FileRestoreEvent> emittedEvents;
	_restorer->GetEventManager().Subscribe([&](const auto& fileRestoreEvent) {
		emittedEvents.push_back(fileRestoreEvent);
	});

	// Act
	const auto lastEvent = _finder->GetLastEventByPath(_sampleFilePath);
	_restorer->Restore(lastEvent, fileRestorePath.ToString());

	// Assert
	const FileRestoreEvent expectedRestoreEvent(lastEvent, fileRestorePath, FileRestoreEventAction::Restored);
	EXPECT_THAT(emittedEvents, ::testing::ElementsAre(expectedRestoreEvent));
	EXPECT_THAT(_sampleFilePath, HasSameFileContents(fileRestorePath));
}

TEST_F(FileRestorerIntegrationTest, Restore_IgnoresUnsupportedEvents)
{
	// Arrange
	std::vector<FileEvent> eventsToRestore = {
		DirectoryEvent(fs::NativePath(R"(C:\hey baby)"), FileEventAction::ChangedRemoved),
		RegularFileEvent(fs::NativePath(R"(C:\hey other)"), boost::none, FileEventAction::Unsupported)
	};

	// Act
	_restorer->Restore(eventsToRestore, _restorePath.ToString());

	// Assert
	const std::vector<FileRestoreEvent> expectedEmittedEvents = {
		FileRestoreEvent(eventsToRestore[0], _restorePath / R"(C\hey baby)", FileRestoreEventAction::UnsupportedFileEvent),
		FileRestoreEvent(eventsToRestore[1], _restorePath / R"(C\hey other)", FileRestoreEventAction::UnsupportedFileEvent),
	};
	EXPECT_THAT(_restorer->GetEmittedEvents(), ::testing::UnorderedElementsAreArray(expectedEmittedEvents));
}

TEST_F(FileRestorerIntegrationTest, Restore_Success)
{
	// Arrange
	_adder->Add(_sampleBasePath.ToString());

	// Act
	_restorer->Restore(_adder->GetEmittedEvents(), _restorePath.ToString());

	// Assert
	const auto sampleBasePathRestored = _restorePath.AppendFullCopy(_sampleBasePath).EnsureTrailingSlash();
	EXPECT_TRUE(fs::Exists(sampleBasePathRestored));
	const auto sampleSubDirectoryRestored = _restorePath.AppendFullCopy(_sampleSubDirectory).EnsureTrailingSlash();
	EXPECT_TRUE(fs::Exists(sampleSubDirectoryRestored));
	const auto sampleFilePathRestored = _restorePath.AppendFullCopy(_sampleFilePath);
	EXPECT_THAT(_sampleFilePath, HasSameFileContents(sampleFilePathRestored));
	const auto sampleSubFilePathRestored = _restorePath.AppendFullCopy(_sampleSubFilePath);
	EXPECT_THAT(_sampleSubFilePath, HasSameFileContents(sampleSubFilePathRestored));
}

TEST_F(FileRestorerIntegrationTest, Restore_ToRelativeDirectorySuccess)
{
	// Arrange
	const auto restorePath = fs::NativeFromBoostPath(GetUniqueTempPath());
	fs::CreateDirectories(restorePath);
	_adder->Add(_sampleBasePath.ToString());

	// Act
	test_utility::ScopedWorkingDirectory workingDirectory(restorePath);
	_restorer->Restore(_adder->GetEmittedEvents(), ".");

	// Assert
	const auto sampleBasePathRestored = restorePath.AppendFullCopy(_sampleBasePath).EnsureTrailingSlash();
	EXPECT_TRUE(fs::Exists(sampleBasePathRestored));
	const auto sampleSubDirectoryRestored = restorePath.AppendFullCopy(_sampleSubDirectory).EnsureTrailingSlash();
	EXPECT_TRUE(fs::Exists(sampleSubDirectoryRestored));
	const auto sampleFilePathRestored = restorePath.AppendFullCopy(_sampleFilePath);
	EXPECT_THAT(_sampleFilePath, HasSameFileContents(sampleFilePathRestored));
	const auto sampleSubFilePathRestored = restorePath.AppendFullCopy(_sampleSubFilePath);
	EXPECT_THAT(_sampleSubFilePath, HasSameFileContents(sampleSubFilePathRestored));
}

TEST_F(FileRestorerIntegrationTest, Restore_SkipsExistingFiles)
{
	// Arrange
	_adder->Add(_sampleBasePath.ToString());

	const auto sampleSubFilePathRestored = _restorePath.AppendFullCopy(_sampleSubFilePath);
	fs::CreateDirectories(sampleSubFilePathRestored.ParentPathCopy());
	CreateFile(sampleSubFilePathRestored, "Something else");

	// Act
	_restorer->Restore(_adder->GetEmittedEvents(), _restorePath.ToString());

	// Assert
	const auto emittedEvents = _restorer->GetEmittedEvents();

	const auto sampleBasePathRestored = _restorePath.AppendFullCopy(_sampleBasePath).EnsureTrailingSlash();
	EXPECT_TRUE(fs::IsDirectory(sampleBasePathRestored));
	EXPECT_THAT(emittedEvents, ::testing::Contains(::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(sampleBasePathRestored))));

	const auto sampleSubDirectoryRestored = _restorePath.AppendFullCopy(_sampleSubDirectory).EnsureTrailingSlash();
	EXPECT_TRUE(fs::IsDirectory(sampleSubDirectoryRestored));
	EXPECT_THAT(emittedEvents, ::testing::Contains(::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(sampleSubDirectoryRestored))));

	const auto sampleFilePathRestored = _restorePath.AppendFullCopy(_sampleFilePath);
	EXPECT_THAT(_sampleFilePath, HasSameFileContents(sampleFilePathRestored));
	EXPECT_THAT(emittedEvents, ::testing::Contains(::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(sampleFilePathRestored))));

	EXPECT_THAT(_sampleSubFilePath, ::testing::Not(HasSameFileContents(sampleSubFilePathRestored)));
	EXPECT_THAT(emittedEvents, ::testing::Contains(
		::testing::AllOf(
			::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(sampleSubFilePathRestored)),
			::testing::Field(&FileRestoreEvent::action, ::testing::Eq(FileRestoreEventAction::Skipped))
		)
	));
}

TEST_F(FileRestorerIntegrationTest, Restore_HandlesFileDirectoryClash)
{
	// Arrange
	_adder->Add(_sampleBasePath.ToString());

	// Create a file in the spot of one of the directories being restored
	const auto sampleSubDirectoryRestored = _restorePath.AppendFullCopy(_sampleSubDirectory);
	fs::CreateDirectories(sampleSubDirectoryRestored.ParentPathCopy());
	CreateFile(sampleSubDirectoryRestored, "Uhoh");

	// Act
	_restorer->Restore(_adder->GetEmittedEvents(), _restorePath.ToString());

	// Assert
	const auto emittedEvents = _restorer->GetEmittedEvents();

	const auto sampleBasePathRestored = _restorePath.AppendFullCopy(_sampleBasePath).EnsureTrailingSlash();
	EXPECT_TRUE(fs::Exists(sampleBasePathRestored));

	EXPECT_TRUE(fs::IsRegularFile(sampleSubDirectoryRestored));
	EXPECT_THAT(emittedEvents, ::testing::Contains(
		::testing::AllOf(
			::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(sampleSubDirectoryRestored.EnsureTrailingSlashCopy())),
			::testing::Field(&FileRestoreEvent::action, ::testing::Eq(FileRestoreEventAction::FailedToCreateDirectory))
		)
	));

	const auto sampleFilePathRestored = _restorePath.AppendFullCopy(_sampleFilePath);
	EXPECT_THAT(_sampleFilePath, HasSameFileContents(sampleFilePathRestored));

	const auto sampleSubFilePathRestored = _restorePath.AppendFullCopy(_sampleSubFilePath);
	EXPECT_TRUE(!fs::Exists(sampleSubFilePathRestored));
	EXPECT_THAT(emittedEvents, ::testing::Contains(
		::testing::AllOf(
			::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(sampleSubFilePathRestored)),
			::testing::Field(&FileRestoreEvent::action, ::testing::Eq(FileRestoreEventAction::FailedToCreateDirectory))
		)
	));
}

}
}
}
}
