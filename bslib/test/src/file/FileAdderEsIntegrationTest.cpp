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
	CreateFile(filePath, "hello");

	// Act
	_adder->Add(filePath);

	// Assert
	EXPECT_TRUE(_finder->FindLastEventByPath(filePath));
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
	EXPECT_FALSE(_finder->FindLastEventByPath(filePath));
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
	EXPECT_TRUE(_finder->FindLastEventByPath(path));
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
	CreateFile(filePath, "hello");

	// Act
	_adder->Add(path);
	_uow->Commit();

	// Assert
	EXPECT_TRUE(_finder->FindLastEventByPath(path));
	EXPECT_TRUE(_finder->FindLastEventByPath(deepDirectory));
	EXPECT_TRUE(_finder->FindLastEventByPath(filePath));

	{
		auto uow2 = _forest->CreateUnitOfWork();
		auto finder = uow2->CreateFileFinder();
		EXPECT_TRUE(finder->FindLastEventByPath(path));
		EXPECT_TRUE(finder->FindLastEventByPath(deepDirectory));

		const auto fileEvent = finder->FindLastEventByPath(filePath);
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
	CreateFile(filePath, "hell");
	CreateFile(deepFilePath, "hello");
	adder2->Add(path);
	uow2->Commit();

	// Assert
	{
		auto uow3 = _forest->CreateUnitOfWork();
		auto finder3 = uow3->CreateFileFinder();
		EXPECT_TRUE(finder3->FindLastEventByPath(path));
		EXPECT_TRUE(finder3->FindLastEventByPath(deepDirectory));
		EXPECT_TRUE(finder3->FindLastEventByPath(deepFilePath));
		EXPECT_TRUE(finder3->FindLastEventByPath(filePath));

		// Deep file
		{
			const auto fileEvent = finder3->FindLastEventByPath(deepFilePath);
			ASSERT_TRUE(fileEvent);
			EXPECT_EQ(helloBytes, uow3->GetBlob(fileEvent->contentBlobAddress.value()));
		}

		// Updated file
		{
			const auto fileEvent = finder3->FindLastEventByPath(filePath);
			ASSERT_TRUE(fileEvent);
			EXPECT_EQ(hellBytes, uow3->GetBlob(fileEvent->contentBlobAddress.value()));
		}
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

	const std::vector<FileEvent> expectedEvents = {
		FileEvent(path, boost::none, FileEventAction::Added),
		FileEvent(fooPath, boost::none, FileEventAction::Added),
		// The old case isn't checked, so the same file may be in here twice
		FileEvent(samsonPath, fileAddress, FileEventAction::Added),
		FileEvent(samsonUpperPath, fileAddress, FileEventAction::Added),
	};

	_adder->Add(path);

	// Recreate "Samson.txt", and note this should be tracked as completely new file
	boost::filesystem::remove(samsonPath);
	CreateFile(samsonUpperPath, "samson was here");

	// Act
	_adder->Add(path);
	_uow->Commit();

	// Assert
	auto uow2 = _forest->CreateUnitOfWork();
	auto finder2 = uow2->CreateFileFinder();
	const auto& result = finder2->GetLastEventsStartingWithPath(path);
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

	// Recreate "Samson.txt", and note this should be tracked as completely new file
	boost::filesystem::remove(samsonPath);
	const auto fileAddress = CreateFile(samsonPath, "samson was here with some new content");

	// Also delete bar
	boost::filesystem::remove_all(barPath);

	// Add a new top level directory
	const auto fizzPath = DirectoryPath(path / "fizz");
	boost::filesystem::create_directories(fizzPath);

	const std::vector<FileEvent> expectedEvents = {
		FileEvent(path, boost::none, FileEventAction::Added),
		FileEvent(fooPath, boost::none, FileEventAction::Added),
		FileEvent(samsonPath, fileAddress, FileEventAction::Modified),
		FileEvent(sakoPath, sakoContentAddress, FileEventAction::Removed),
		FileEvent(barPath, boost::none, FileEventAction::Removed),
		FileEvent(fizzPath, boost::none, FileEventAction::Added),
	};

	// Act
	_adder->Add(path);
	_uow->Commit();

	// Assert
	auto uow2 = _forest->CreateUnitOfWork();
	auto finder2 = uow2->CreateFileFinder();
	const auto& result = finder2->GetLastEventsStartingWithPath(path);
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

	const std::vector<FileEvent> expectedEvents = {
		FileEvent(path, boost::none, FileEventAction::Added),
		FileEvent(fooDirectoryPath, boost::none, FileEventAction::Removed),
		FileEvent(fooFilePath, fileAddress, FileEventAction::Added)
	};

	// Act
	_adder->Add(path);
	_uow->Commit();

	// Assert
	auto uow2 = _forest->CreateUnitOfWork();
	auto finder2 = uow2->CreateFileFinder();
	const auto& result = finder2->GetLastEventsStartingWithPath(path);
	EXPECT_THAT(expectedEvents, ::testing::UnorderedElementsAreArray(result | boost::adaptors::map_values));
}

}
}
}
}
