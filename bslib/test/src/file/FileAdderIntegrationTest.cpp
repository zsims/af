#include "bslib/file/exceptions.hpp"
#include "bslib/file/fs/operations.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"
#include "file/test_utility/ScopedExclusiveFileAccess.hpp"
#include "bslib_test_util/TestBase.hpp"
#include "bslib_test_util/gtest_boost_filesystem_fix.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/adaptor/map.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <functional>
#include <memory>

namespace af {
namespace bslib {
namespace file {
namespace test {

class FileAdderIntegrationTest : public bslib_test_util::TestBase
{
protected:
	FileAdderIntegrationTest()
		: _backupRunId(Uuid::Create())
	{
		_testBackup.Create();
		_uow = _testBackup.GetBackup().CreateUnitOfWork();
		_adder = _uow->CreateFileAdder(_backupRunId);
		_finder = _uow->CreateFileFinder();
	}

	const Uuid _backupRunId;
	std::unique_ptr<UnitOfWork> _uow;
	std::unique_ptr<FileAdder> _adder;
	std::unique_ptr<FileFinder> _finder;
};

TEST_F(FileAdderIntegrationTest, Add_SuccessWithFile)
{
	// Arrange
	const auto filePath = GetUniqueExtendedTempPath();
	const auto fileAddress = WriteFile(filePath, "hello");

	std::vector<FileEvent> emittedEvents;
	_adder->GetEventManager().Subscribe([&](const auto& fileEvent) {
		emittedEvents.push_back(fileEvent);
	});
	// Act
	_adder->Add(filePath.ToString());

	// Assert
	const auto expectedEmittedEvent = RegularFileEvent(_backupRunId, filePath, fileAddress, FileEventAction::ChangedAdded);
	EXPECT_TRUE(_finder->FindLastChangedEventByPath(filePath));
	EXPECT_THAT(emittedEvents, ::testing::ElementsAre(expectedEmittedEvent));

	const auto pathSegments = filePath.GetIntermediatePaths();
	for (const auto& segment : pathSegments)
	{
		EXPECT_TRUE(_finder->IsKnownPath(segment));
	}
}

TEST_F(FileAdderIntegrationTest, Add_ConvertsForwardSlashes)
{
	// Arrange
	const auto filePath = GetUniqueExtendedTempPath();
	const auto fileAddress = WriteFile(filePath, "hello");

	// Act
	const auto forwardSlashFilePath = boost::replace_last_copy(filePath.ToString(), "\\", "/");
	_adder->Add(forwardSlashFilePath);

	// Assert
	const auto expectedEmittedEvent = RegularFileEvent(_backupRunId, filePath, fileAddress, FileEventAction::ChangedAdded);
	EXPECT_TRUE(_finder->FindLastChangedEventByPath(filePath));
	EXPECT_THAT(_adder->GetEmittedEvents(), ::testing::ElementsAre(expectedEmittedEvent));
}

TEST_F(FileAdderIntegrationTest, Add_ResolvesFullPath)
{
	// Arrange
	const auto folderName = "folder";
	const auto workingPath = (GetUniqueExtendedTempPath() / folderName).EnsureTrailingSlash();
	fs::CreateDirectories(workingPath);
	const auto fileAddress = WriteFile(workingPath / "hi.dat", "hello");
	const auto sourcePath = workingPath / ".." / folderName;

	// Act
	_adder->Add(sourcePath.ToString());

	// Assert
	const auto& emittedEvents = _adder->GetEmittedEvents();
	const auto expectedFileEvent = RegularFileEvent(_backupRunId, workingPath / "hi.dat", fileAddress, FileEventAction::ChangedAdded);
	const auto expectedDirectoryEvent = DirectoryEvent(_backupRunId, workingPath, FileEventAction::ChangedAdded);
	EXPECT_TRUE(_finder->FindLastChangedEventByPath(workingPath));
	EXPECT_THAT(emittedEvents, ::testing::ElementsAre(expectedDirectoryEvent, expectedFileEvent));
}

TEST_F(FileAdderIntegrationTest, Add_SkipsLockedFile)
{
	// Arrange
	const auto filePath = GetUniqueExtendedTempPath();
	WriteFile(filePath, "hello");

	test_utility::ScopedExclusiveFileAccess exclusiveAccess(filePath);

	// Act
	_adder->Add(filePath.ToExtendedString());

	// Assert
	const FileEvent expectedEmittedEvent(_backupRunId, filePath, FileType::RegularFile, boost::none, FileEventAction::FailedToRead);
	EXPECT_FALSE(_finder->FindLastChangedEventByPath(filePath));
	EXPECT_THAT(_adder->GetEmittedEvents(), ::testing::ElementsAre(expectedEmittedEvent));
}

TEST_F(FileAdderIntegrationTest, Add_RecordsAllStates)
{
	// Arrange
	const auto directoryPath = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	fs::CreateDirectories(directoryPath);

	const auto lockedFilePath = directoryPath / "locked.dat";
	WriteFile(lockedFilePath, "hello");
	test_utility::ScopedExclusiveFileAccess exclusiveAccess(lockedFilePath);

	const auto preExistingPath = directoryPath / "already-here.dat";
	const auto preExistingAddress = WriteFile(preExistingPath, "heybby");
	_adder->Add(preExistingPath.ToString());

	// Act
	_adder->Add(directoryPath.ToString());

	// Assert
	const std::vector<FileEvent> expectedAllEvents = {
		RegularFileEvent(_backupRunId, preExistingPath, preExistingAddress, FileEventAction::ChangedAdded),
		DirectoryEvent(_backupRunId, directoryPath, FileEventAction::ChangedAdded),
		RegularFileEvent(_backupRunId, preExistingPath, preExistingAddress, FileEventAction::Unchanged),
		RegularFileEvent(_backupRunId, lockedFilePath, boost::none, FileEventAction::FailedToRead)
	};
	const auto& allEvents = _finder->GetAllEvents();
	EXPECT_EQ(allEvents, expectedAllEvents);
}

TEST_F(FileAdderIntegrationTest, Add_FailIfNotExist)
{
	// Arrange
	const auto path = GetUniqueExtendedTempPath().EnsureTrailingSlash();

	// Act
	// Assert
	ASSERT_THROW(_adder->Add(path.ToString()), PathNotFoundException);
}

TEST_F(FileAdderIntegrationTest, Add_EmptyDirectory)
{
	// Arrange
	const auto path = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	fs::CreateDirectories(path);

	// Act
	_adder->Add(path.ToString());

	// Assert
	const FileEvent expectedEmittedEvent(_backupRunId, path, FileType::Directory, boost::none, FileEventAction::ChangedAdded);
	EXPECT_TRUE(_finder->FindLastChangedEventByPath(path));
	EXPECT_THAT(_adder->GetEmittedEvents(), ::testing::ElementsAre(expectedEmittedEvent));
}

TEST_F(FileAdderIntegrationTest, Add_SuccessWithDirectory)
{
	// Arrange
	const auto path = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	fs::CreateDirectories(path);
	const auto deepDirectory = (path / "deep").EnsureTrailingSlash();
	fs::CreateDirectories(deepDirectory);
	const auto filePath = path / "file.dat";
	const std::vector<uint8_t> helloBytes = { 104, 101, 108, 108, 111 };
	const auto fileAddress = WriteFile(filePath, "hello");

	// Act
	_adder->Add(path.ToString());
	_uow->Commit();

	// Assert
	const std::vector<FileEvent> expectedEmittedEvents = {
		DirectoryEvent(_backupRunId, path, FileEventAction::ChangedAdded),
		DirectoryEvent(_backupRunId, deepDirectory, FileEventAction::ChangedAdded),
		RegularFileEvent(_backupRunId, filePath, fileAddress, FileEventAction::ChangedAdded)
	};
	EXPECT_THAT(_adder->GetEmittedEvents(), ::testing::UnorderedElementsAreArray(expectedEmittedEvents));
	EXPECT_TRUE(_finder->FindLastChangedEventByPath(path));
	EXPECT_TRUE(_finder->FindLastChangedEventByPath(deepDirectory));
	EXPECT_TRUE(_finder->FindLastChangedEventByPath(filePath));

	{
		auto uow2 = _testBackup.GetBackup().CreateUnitOfWork();
		auto finder = uow2->CreateFileFinder();
		EXPECT_TRUE(finder->FindLastChangedEventByPath(path));
		EXPECT_TRUE(finder->FindLastChangedEventByPath(deepDirectory));

		const auto fileEvent = finder->FindLastChangedEventByPath(filePath);
		ASSERT_TRUE(fileEvent);
		EXPECT_EQ(helloBytes, uow2->GetBlob(fileEvent->contentBlobAddress.value()));
	}
}

TEST_F(FileAdderIntegrationTest, Add_ExistingSuccessWithDirectory)
{
	// Arrange
	const auto path = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	fs::CreateDirectories(path);
	const auto deepDirectory = (path / "deep").EnsureTrailingSlash();
	fs::CreateDirectories(deepDirectory);
	const auto filePath = path / "file.dat";
	const auto deepFilePath = deepDirectory / "foo.dat";
	const std::vector<uint8_t> helloBytes = { 104, 101, 108, 108, 111 };
	const std::vector<uint8_t> hellBytes = { 104, 101, 108, 108 };
	WriteFile(filePath, "hello");
	_adder->Add(path.ToString());
	_uow->Commit();

	// Act
	auto uow2 = _testBackup.GetBackup().CreateUnitOfWork();
	auto adder2 = uow2->CreateFileAdder(_backupRunId);
	const auto updatedFileAddress = WriteFile(filePath, "hell");
	const auto deepFileAddress = WriteFile(deepFilePath, "hello");
	adder2->Add(path.ToString());
	uow2->Commit();

	// Assert
	const std::vector<FileEvent> expectedSecondEmittedEvents = {
		// Directories are unchanged, so they won't be here
		RegularFileEvent(_backupRunId, filePath, updatedFileAddress, FileEventAction::ChangedModified),
		RegularFileEvent(_backupRunId, deepFilePath, deepFileAddress, FileEventAction::ChangedAdded)
	};
	EXPECT_THAT(adder2->GetEmittedEvents(), ::testing::UnorderedElementsAreArray(expectedSecondEmittedEvents));
	auto uow3 = _testBackup.GetBackup().CreateUnitOfWork();
	auto finder3 = uow3->CreateFileFinder();
	EXPECT_TRUE(finder3->FindLastChangedEventByPath(path));
	EXPECT_TRUE(finder3->FindLastChangedEventByPath(deepDirectory));
	EXPECT_TRUE(finder3->FindLastChangedEventByPath(deepFilePath));
	EXPECT_TRUE(finder3->FindLastChangedEventByPath(filePath));

	// Deep file
	{
		const auto fileEvent = finder3->FindLastChangedEventByPath(deepFilePath);
		ASSERT_TRUE(fileEvent);
		EXPECT_EQ(helloBytes, uow3->GetBlob(fileEvent->contentBlobAddress.value()));
	}

	// Updated file
	{
		const auto fileEvent = finder3->FindLastChangedEventByPath(filePath);
		ASSERT_TRUE(fileEvent);
		EXPECT_EQ(hellBytes, uow3->GetBlob(fileEvent->contentBlobAddress.value()));
	}
}

TEST_F(FileAdderIntegrationTest, Add_RespectsCase)
{
	// Arrange
	const auto path = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	const auto fooPath = (path / "Foo").EnsureTrailingSlash();
	fs::CreateDirectories(fooPath);
	const auto samsonPath = fooPath / "samson.txt";
	const auto fileAddress = WriteFile(samsonPath, "samson was here");
	const auto samsonUpperPath = fooPath / "Samson.txt";

	_adder->Add(path.ToString());

	// Recreate "Samson.txt", and note this should be tracked as completely new file
	fs::Remove(samsonPath);
	WriteFile(samsonUpperPath, "samson was here");

	// Act
	_adder->Add(path.ToString());
	_uow->Commit();

	// Assert
	const std::vector<FileEvent> expectedEvents = {
		DirectoryEvent(_backupRunId, path, FileEventAction::ChangedAdded),
		DirectoryEvent(_backupRunId, fooPath, FileEventAction::ChangedAdded),
		// The old case isn't checked, so the same file may be in here twice
		RegularFileEvent(_backupRunId, samsonPath, fileAddress, FileEventAction::ChangedAdded),
		RegularFileEvent(_backupRunId, samsonUpperPath, fileAddress, FileEventAction::ChangedAdded),
	};
	auto uow2 = _testBackup.GetBackup().CreateUnitOfWork();
	auto finder2 = uow2->CreateFileFinder();
	const auto& result = finder2->GetLastChangedEventsUnderPath(path);
	EXPECT_THAT(expectedEvents, ::testing::UnorderedElementsAreArray(result | boost::adaptors::map_values));
}

TEST_F(FileAdderIntegrationTest, Add_DetectsModifications)
{
	// Arrange
	const auto path = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	const auto fooPath = (path / "Foo").EnsureTrailingSlash();
	fs::CreateDirectories(fooPath);
	const auto barPath = (fooPath / "Bar").EnsureTrailingSlash();
	fs::CreateDirectories(barPath);
	const auto samsonPath = fooPath / "samson.txt";
	const auto sakoPath = barPath / "sako.txt";
	WriteFile(samsonPath, "samson was here");
	const auto sakoContentAddress = WriteFile(sakoPath, "sako was here");

	_adder->Add(path.ToString());

	// Recreate "samson.txt", and note this should be tracked as completely new file
	fs::Remove(samsonPath);
	const auto fileAddress = WriteFile(samsonPath, "samson was here with some new content");

	// Also delete bar
	fs::RemoveAll(barPath);

	// Add a new top level directory
	const auto fizzPath = (path / "fizz").EnsureTrailingSlash();
	fs::CreateDirectories(fizzPath);

	// Make not of the number of emitted events so we can work out what events were emitted in the second Add()
	auto beforeCount = _adder->GetEmittedEvents().size();

	// Act
	_adder->Add(path.ToString());
	_uow->Commit();

	// Assert
	const std::vector<FileEvent> expectedEmittedEvents = {
		RegularFileEvent(_backupRunId, samsonPath, fileAddress, FileEventAction::ChangedModified),
		DirectoryEvent(_backupRunId, barPath, FileEventAction::ChangedRemoved),
		RegularFileEvent(_backupRunId, sakoPath, sakoContentAddress, FileEventAction::ChangedRemoved),
		DirectoryEvent(_backupRunId, fizzPath, FileEventAction::ChangedAdded)
	};
	const auto all = _adder->GetEmittedEvents();
	std::vector<FileEvent> newEvents(all.begin() + beforeCount, all.end());
	EXPECT_THAT(newEvents, ::testing::UnorderedElementsAreArray(expectedEmittedEvents));
	auto uow2 = _testBackup.GetBackup().CreateUnitOfWork();
	auto finder2 = uow2->CreateFileFinder();
	const auto& result = finder2->GetLastChangedEventsUnderPath(path);
	const std::vector<FileEvent> expectedEvents = {
		DirectoryEvent(_backupRunId, path, FileEventAction::ChangedAdded),
		DirectoryEvent(_backupRunId, fooPath, FileEventAction::ChangedAdded),
		RegularFileEvent(_backupRunId, samsonPath, fileAddress, FileEventAction::ChangedModified),
		RegularFileEvent(_backupRunId, sakoPath, sakoContentAddress, FileEventAction::ChangedRemoved),
		DirectoryEvent(_backupRunId, barPath, FileEventAction::ChangedRemoved),
		DirectoryEvent(_backupRunId, fizzPath, FileEventAction::ChangedAdded),
	};
	EXPECT_THAT(expectedEvents, ::testing::UnorderedElementsAreArray(result | boost::adaptors::map_values));
}

TEST_F(FileAdderIntegrationTest, Add_HandlesChangeInType)
{
	// Arrange
	const auto path = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	const auto fooDirectoryPath = (path / "Foo").EnsureTrailingSlash();
	const auto fooFilePath = path / "Foo";
	fs::CreateDirectories(fooDirectoryPath);
	_adder->Add(path.ToString());

	// Recreate foo but as a file
	fs::RemoveAll(fooDirectoryPath);
	const auto fileAddress = WriteFile(fooFilePath, "samson was here");

	// Act
	_adder->Add(path.ToString());
	_uow->Commit();

	// Assert
	const std::vector<FileEvent> expectedEvents = {
		DirectoryEvent(_backupRunId, path, FileEventAction::ChangedAdded),
		DirectoryEvent(_backupRunId, fooDirectoryPath, FileEventAction::ChangedRemoved),
		RegularFileEvent(_backupRunId, fooFilePath, fileAddress, FileEventAction::ChangedAdded)
	};
	auto uow2 = _testBackup.GetBackup().CreateUnitOfWork();
	auto finder2 = uow2->CreateFileFinder();
	const auto& result = finder2->GetLastChangedEventsUnderPath(path);
	EXPECT_THAT(expectedEvents, ::testing::UnorderedElementsAreArray(result | boost::adaptors::map_values));
}

}
}
}
}
