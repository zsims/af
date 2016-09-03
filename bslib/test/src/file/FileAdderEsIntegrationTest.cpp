#include "bslib/forest.hpp"
#include "bslib/blob/DirectoryBlobStore.hpp"
#include "bslib/file/DirectoryPath.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"
#include "utility/gtest_boost_filesystem_fix.hpp"
#include "utility/ScopedExclusiveFileAccess.hpp"

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

class FileAdderEsIntegrationTest : public testing::Test
{
protected:
	FileAdderEsIntegrationTest()
		: _forestDbPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.fdb"))
		, _targetPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
	{
		boost::filesystem::create_directories(_targetPath);
		auto blobStore = std::make_unique<blob::DirectoryBlobStore>(_targetPath);
		_forest.reset(new Forest(_forestDbPath.string(), std::move(blobStore)));
		_forest->Create();

		_uow = _forest->CreateUnitOfWork();
		_adder = _uow->CreateFileAdderEs();
		_finder = _uow->CreateFileFinder();
	}

	~FileAdderEsIntegrationTest()
	{
		_adder.reset();
		_uow.reset();
		_forest.reset();

		boost::system::error_code ec;
		boost::filesystem::remove(_forestDbPath, ec);
		boost::filesystem::remove_all(_targetPath, ec);
	}

	BlobAddress CreateFile(const boost::filesystem::path& path, const std::string& content)
	{
		const std::vector<uint8_t> binaryContent(content.begin(), content.end());
		std::ofstream f(path.string(), std::ofstream::out | std::ofstream::binary);
		f.write(reinterpret_cast<const char*>(&binaryContent[0]), binaryContent.size());
		return BlobAddress::CalculateFromContent(binaryContent);
	}

	const boost::filesystem::path _forestDbPath;
	const boost::filesystem::path _targetPath;
	std::unique_ptr<Forest> _forest;
	std::unique_ptr<UnitOfWork> _uow;
	std::unique_ptr<FileAdderEs> _adder;
	std::unique_ptr<FileFinder> _finder;
};

TEST_F(FileAdderEsIntegrationTest, Add_SuccessWithFile)
{
	// Arrange
	const auto filePath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	const std::vector<uint8_t> helloBytes = { 104, 101, 108, 108, 111 };
	const auto fileAddress = CreateFile(filePath, "hello");

	std::vector<FileEvent> emittedEvents;
	_adder->SubscribeToEmit([&](const auto& fileEvent) {
		emittedEvents.push_back(fileEvent);
	});
	// Act
	_adder->Add(filePath);

	// Assert
	const std::vector<FileEvent> expectedEmittedEvents = {
		FileEvent(filePath, fileAddress, FileEventAction::ChangedAdded)
	};
	EXPECT_TRUE(_finder->FindLastChangedEventByPath(filePath));
	EXPECT_THAT(emittedEvents, ::testing::UnorderedElementsAreArray(expectedEmittedEvents));
}

TEST_F(FileAdderEsIntegrationTest, Add_SkipsLockedFile)
{
	// Arrange
	const auto filePath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	const std::vector<uint8_t> helloBytes = { 104, 101, 108, 108, 111 };
	CreateFile(filePath, "hello");

	bslib::test::utility::ScopedExclusiveFileAccess exclusiveAccess(filePath);

	// Act
	_adder->Add(filePath);

	// Assert
	const std::vector<FileEvent> expectedEmittedEvents = {
		FileEvent(filePath, boost::none, FileEventAction::FailedToRead)
	};
	EXPECT_FALSE(_finder->FindLastChangedEventByPath(filePath));
	EXPECT_THAT(_adder->GetEmittedEvents(), ::testing::UnorderedElementsAreArray(expectedEmittedEvents));
}

TEST_F(FileAdderEsIntegrationTest, Add_RecordsAllStates)
{
	// Arrange
	const DirectoryPath directoryPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path());
	boost::filesystem::create_directories(directoryPath);

	const auto lockedFilePath = directoryPath / "locked.dat";
	CreateFile(lockedFilePath, "hello");
	bslib::test::utility::ScopedExclusiveFileAccess exclusiveAccess(lockedFilePath);

	const auto preExistingPath = directoryPath / "already-here.dat";
	const auto preExistingAddress = CreateFile(preExistingPath, "heybby");
	_adder->Add(preExistingPath);

	// Act
	_adder->Add(directoryPath);

	// Assert
	const std::vector<FileEvent> expectedAllEvents = {
		FileEvent(preExistingPath, preExistingAddress, FileEventAction::ChangedAdded),
		FileEvent(directoryPath, boost::none, FileEventAction::ChangedAdded),
		FileEvent(preExistingPath, preExistingAddress, FileEventAction::Unchanged),
		FileEvent(lockedFilePath, boost::none, FileEventAction::FailedToRead)
	};
	const auto& allEvents = _finder->GetAllEvents();
	EXPECT_EQ(allEvents, expectedAllEvents);
}

TEST_F(FileAdderEsIntegrationTest, Add_FailIfNotExist)
{
	// Arrange
	const DirectoryPath path(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path());

	// Act
	// Assert
	ASSERT_THROW(_adder->Add(path), PathNotFoundException);
}

TEST_F(FileAdderEsIntegrationTest, Add_EmptyDirectory)
{
	// Arrange
	const DirectoryPath path(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path());
	boost::filesystem::create_directories(path);

	// Act
	_adder->Add(path);

	// Assert
	const std::vector<FileEvent> expectedEmittedEvents = {
		FileEvent(DirectoryPath(path), boost::none, FileEventAction::ChangedAdded)
	};
	EXPECT_TRUE(_finder->FindLastChangedEventByPath(path));
	EXPECT_THAT(_adder->GetEmittedEvents(), ::testing::UnorderedElementsAreArray(expectedEmittedEvents));
}

TEST_F(FileAdderEsIntegrationTest, Add_SuccessWithDirectory)
{
	// Arrange
	const DirectoryPath path(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path());
	boost::filesystem::create_directories(path);
	const DirectoryPath deepDirectory(path / boost::filesystem::unique_path());
	boost::filesystem::create_directories(deepDirectory);
	const auto filePath = path / "file.dat";
	const std::vector<uint8_t> helloBytes = { 104, 101, 108, 108, 111 };
	const auto fileAddress = CreateFile(filePath, "hello");

	// Act
	_adder->Add(path);
	_uow->Commit();

	// Assert
	const std::vector<FileEvent> expectedEmittedEvents = {
		FileEvent(path, boost::none, FileEventAction::ChangedAdded),
		FileEvent(deepDirectory, boost::none, FileEventAction::ChangedAdded),
		FileEvent(filePath, fileAddress, FileEventAction::ChangedAdded)
	};
	EXPECT_THAT(_adder->GetEmittedEvents(), ::testing::UnorderedElementsAreArray(expectedEmittedEvents));
	EXPECT_TRUE(_finder->FindLastChangedEventByPath(path));
	EXPECT_TRUE(_finder->FindLastChangedEventByPath(deepDirectory));
	EXPECT_TRUE(_finder->FindLastChangedEventByPath(filePath));

	{
		auto uow2 = _forest->CreateUnitOfWork();
		auto finder = uow2->CreateFileFinder();
		EXPECT_TRUE(finder->FindLastChangedEventByPath(path));
		EXPECT_TRUE(finder->FindLastChangedEventByPath(deepDirectory));

		const auto fileEvent = finder->FindLastChangedEventByPath(filePath);
		ASSERT_TRUE(fileEvent);
		EXPECT_EQ(helloBytes, uow2->GetBlob(fileEvent->contentBlobAddress.value()));
	}
}

TEST_F(FileAdderEsIntegrationTest, Add_ExistingSuccessWithDirectory)
{
	// Arrange
	const DirectoryPath path(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path());
	boost::filesystem::create_directories(path);
	const DirectoryPath deepDirectory(path / boost::filesystem::unique_path());
	boost::filesystem::create_directories(deepDirectory);
	const auto filePath = path / "file.dat";
	const auto deepFilePath = deepDirectory / "foo.dat";
	const std::vector<uint8_t> helloBytes = { 104, 101, 108, 108, 111 };
	const std::vector<uint8_t> hellBytes = { 104, 101, 108, 108 };
	CreateFile(filePath, "hello");
	_adder->Add(path);
	_uow->Commit();

	// Act
	auto uow2 = _forest->CreateUnitOfWork();
	auto adder2 = uow2->CreateFileAdderEs();
	const auto updatedFileAddress = CreateFile(filePath, "hell");
	const auto deepFileAddress = CreateFile(deepFilePath, "hello");
	adder2->Add(path);
	uow2->Commit();

	// Assert
	const std::vector<FileEvent> expectedSecondEmittedEvents = {
		// Directories are unchanged, so they won't be here
		FileEvent(filePath, updatedFileAddress, FileEventAction::ChangedModified),
		FileEvent(deepFilePath, deepFileAddress, FileEventAction::ChangedAdded)
	};
	EXPECT_THAT(adder2->GetEmittedEvents(), ::testing::UnorderedElementsAreArray(expectedSecondEmittedEvents));
	auto uow3 = _forest->CreateUnitOfWork();
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

TEST_F(FileAdderEsIntegrationTest, Add_RespectsCase)
{
	// Arrange
	const DirectoryPath path(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path());
	const DirectoryPath fooPath(path / "Foo");
	boost::filesystem::create_directories(fooPath);
	const auto samsonPath = fooPath / "samson.txt";
	const auto fileAddress = CreateFile(samsonPath, "samson was here");
	const auto samsonUpperPath = fooPath / "Samson.txt";

	_adder->Add(path);

	// Recreate "Samson.txt", and note this should be tracked as completely new file
	boost::filesystem::remove(samsonPath);
	CreateFile(samsonUpperPath, "samson was here");

	// Act
	_adder->Add(path);
	_uow->Commit();

	// Assert
	const std::vector<FileEvent> expectedEvents = {
		FileEvent(path, boost::none, FileEventAction::ChangedAdded),
		FileEvent(fooPath, boost::none, FileEventAction::ChangedAdded),
		// The old case isn't checked, so the same file may be in here twice
		FileEvent(samsonPath, fileAddress, FileEventAction::ChangedAdded),
		FileEvent(samsonUpperPath, fileAddress, FileEventAction::ChangedAdded),
	};
	auto uow2 = _forest->CreateUnitOfWork();
	auto finder2 = uow2->CreateFileFinder();
	const auto& result = finder2->GetLastChangedEventsStartingWithPath(path);
	EXPECT_THAT(expectedEvents, ::testing::UnorderedElementsAreArray(result | boost::adaptors::map_values));
}

TEST_F(FileAdderEsIntegrationTest, Add_DetectsModifications)
{
	// Arrange
	const DirectoryPath path(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path());
	const DirectoryPath fooPath(path / "Foo");
	boost::filesystem::create_directories(fooPath);
	const DirectoryPath barPath(fooPath / "Bar");
	boost::filesystem::create_directories(barPath);
	const auto samsonPath = fooPath / "samson.txt";
	const auto sakoPath = barPath / "sako.txt";
	CreateFile(samsonPath, "samson was here");
	const auto sakoContentAddress = CreateFile(sakoPath, "sako was here");

	_adder->Add(path);

	// Recreate "samson.txt", and note this should be tracked as completely new file
	boost::filesystem::remove(samsonPath);
	const auto fileAddress = CreateFile(samsonPath, "samson was here with some new content");

	// Also delete bar
	boost::filesystem::remove_all(barPath);

	// Add a new top level directory
	const auto fizzPath = DirectoryPath(path / "fizz");
	boost::filesystem::create_directories(fizzPath);

	// Make not of the number of emitted events so we can work out what events were emitted in the second Add()
	auto beforeCount = _adder->GetEmittedEvents().size();

	// Act
	_adder->Add(path);
	_uow->Commit();

	// Assert
	const std::vector<FileEvent> expectedEmittedEvents = {
		FileEvent(samsonPath, fileAddress, FileEventAction::ChangedModified),
		FileEvent(barPath, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(sakoPath, sakoContentAddress, FileEventAction::ChangedRemoved),
		FileEvent(fizzPath, boost::none, FileEventAction::ChangedAdded)
	};
	const auto all = _adder->GetEmittedEvents();
	std::vector<FileEvent> newEvents(all.begin() + beforeCount, all.end());
	EXPECT_THAT(newEvents, ::testing::UnorderedElementsAreArray(expectedEmittedEvents));
	auto uow2 = _forest->CreateUnitOfWork();
	auto finder2 = uow2->CreateFileFinder();
	const auto& result = finder2->GetLastChangedEventsStartingWithPath(path);
	const std::vector<FileEvent> expectedEvents = {
		FileEvent(path, boost::none, FileEventAction::ChangedAdded),
		FileEvent(fooPath, boost::none, FileEventAction::ChangedAdded),
		FileEvent(samsonPath, fileAddress, FileEventAction::ChangedModified),
		FileEvent(sakoPath, sakoContentAddress, FileEventAction::ChangedRemoved),
		FileEvent(barPath, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(fizzPath, boost::none, FileEventAction::ChangedAdded),
	};
	EXPECT_THAT(expectedEvents, ::testing::UnorderedElementsAreArray(result | boost::adaptors::map_values));
}

TEST_F(FileAdderEsIntegrationTest, Add_HandlesChangeInType)
{
	// Arrange
	const DirectoryPath path(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path());
	const DirectoryPath fooDirectoryPath(path / "Foo");
	const auto fooFilePath = path / "Foo";
	boost::filesystem::create_directories(fooDirectoryPath);
	_adder->Add(path);

	// Recreate foo but as a file
	boost::filesystem::remove_all(fooDirectoryPath);
	const auto fileAddress = CreateFile(fooFilePath, "samson was here");

	// Act
	_adder->Add(path);
	_uow->Commit();

	// Assert
	const std::vector<FileEvent> expectedEvents = {
		FileEvent(path, boost::none, FileEventAction::ChangedAdded),
		FileEvent(fooDirectoryPath, boost::none, FileEventAction::ChangedRemoved),
		FileEvent(fooFilePath, fileAddress, FileEventAction::ChangedAdded)
	};
	auto uow2 = _forest->CreateUnitOfWork();
	auto finder2 = uow2->CreateFileFinder();
	const auto& result = finder2->GetLastChangedEventsStartingWithPath(path);
	EXPECT_THAT(expectedEvents, ::testing::UnorderedElementsAreArray(result | boost::adaptors::map_values));
}

}
}
}
}
