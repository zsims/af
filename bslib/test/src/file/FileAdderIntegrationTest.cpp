#include "bslib/forest.hpp"
#include "bslib/blob/DirectoryBlobStore.hpp"
#include "bslib/file/FileObjectRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"
#include "utility/gtest_boost_filesystem_fix.hpp"
#include "utility/ScopedExclusiveFileAccess.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace file {
namespace test {

class FileAdderIntegrationTest : public testing::Test
{
protected:
	FileAdderIntegrationTest()
		: _forestDbPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.fdb"))
		, _targetPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
	{
		boost::filesystem::create_directories(_targetPath);
		auto blobStore = std::make_unique<blob::DirectoryBlobStore>(_targetPath);
		_forest.reset(new Forest(_forestDbPath.string(), std::move(blobStore)));
		_forest->Create();

		_uow = _forest->CreateUnitOfWork();
		_adder = _uow->CreateFileAdder();
		_finder = _uow->CreateFileFinder();
	}

	~FileAdderIntegrationTest()
	{
		_adder.reset();
		_uow.reset();
		_forest.reset();

		boost::system::error_code ec;
		boost::filesystem::remove(_forestDbPath, ec);
		boost::filesystem::remove_all(_targetPath, ec);
	}

	void CreateFile(const boost::filesystem::path& path, const std::string& content)
	{
		std::ofstream f(path.string(), std::ofstream::out);
		f << content;
	}

	const boost::filesystem::path _forestDbPath;
	const boost::filesystem::path _targetPath;
	std::unique_ptr<Forest> _forest;
	std::unique_ptr<UnitOfWork> _uow;
	std::unique_ptr<FileAdder> _adder;
	std::unique_ptr<FileFinder> _finder;
};

TEST_F(FileAdderIntegrationTest, AddFile)
{
	// Arrange
	// Act
	const auto address = _adder->Add("/here", { 1, 2, 3 });
	_uow->Commit();

	// Assert
	{
		auto uow2 = _forest->CreateUnitOfWork();
		auto finder = uow2->CreateFileFinder();
		EXPECT_TRUE(finder->FindObjectByAddress(address));
		EXPECT_TRUE(finder->FindReference("/here"));
	}
}

TEST_F(FileAdderIntegrationTest, Add_SuccessWithFile)
{
	// Arrange
	const auto filePath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	const std::vector<uint8_t> helloBytes = { 104, 101, 108, 108, 111 };
	CreateFile(filePath, "hello");

	// Act
	_adder->Add(filePath);

	// Assert
	const auto& added = _adder->GetAddedPaths();
	EXPECT_THAT(added, ::testing::Contains(filePath));
	EXPECT_TRUE(_finder->FindReference(filePath.string()));
}

TEST_F(FileAdderIntegrationTest, Add_SkipsLockedFile)
{
	// Arrange
	const auto filePath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	const std::vector<uint8_t> helloBytes = { 104, 101, 108, 108, 111 };
	CreateFile(filePath, "hello");

	bslib::test::utility::ScopedExclusiveFileAccess exclusiveAccess(filePath);

	// Act
	_adder->Add(filePath);

	// Assert
	const auto& added = _adder->GetAddedPaths();
	EXPECT_THAT(added, ::testing::Not(::testing::Contains(filePath)));
	const auto& skipped = _adder->GetSkippedPaths();
	EXPECT_THAT(skipped, ::testing::Contains(filePath));
}

TEST_F(FileAdderIntegrationTest, Add_FailIfNotExist)
{
	// Arrange
	const auto path = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();

	// Act
	// Assert
	ASSERT_THROW(_adder->Add(path), PathNotFoundException);
}

TEST_F(FileAdderIntegrationTest, Add_EmptyDirectory)
{
	// Arrange
	auto path = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	boost::filesystem::create_directories(path);

	// Act
	_adder->Add(path);

	// Assert
	const auto& added = _adder->GetAddedPaths();
	EXPECT_THAT(added, ::testing::Contains(path));
	const auto& skipped = _adder->GetSkippedPaths();
	EXPECT_THAT(skipped, ::testing::Not(::testing::Contains(path)));
}

TEST_F(FileAdderIntegrationTest, Add_SuccessWithDirectory)
{
	// Arrange
	const auto path = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	boost::filesystem::create_directories(path);
	auto deepDirectory = path / boost::filesystem::unique_path();
	boost::filesystem::create_directories(deepDirectory);
	const auto filePath = path / "file.dat";
	const std::vector<uint8_t> helloBytes = { 104, 101, 108, 108, 111 };
	CreateFile(filePath, "hello");

	// Act
	_adder->Add(path);
	_uow->Commit();

	// Assert
	const auto& added = _adder->GetAddedPaths();
	EXPECT_THAT(added, ::testing::Contains(path));
	EXPECT_THAT(added, ::testing::Contains(deepDirectory));
	EXPECT_THAT(added, ::testing::Contains(filePath));

	{
		auto uow2 = _forest->CreateUnitOfWork();
		auto finder = uow2->CreateFileFinder();
		EXPECT_TRUE(finder->FindReference(path));
		EXPECT_TRUE(finder->FindReference(deepDirectory));

		const auto fileRef = finder->FindReference(filePath);
		EXPECT_TRUE(fileRef);
		const auto fileObject = finder->GetObjectByAddress(fileRef->fileObjectAddress);
		EXPECT_EQ(helloBytes, uow2->GetBlob(fileObject.contentBlobAddress.value()));
	}
}

TEST_F(FileAdderIntegrationTest, Add_ExistingSuccessWithDirectory)
{
	// Arrange
	const auto path = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	boost::filesystem::create_directories(path);
	auto deepDirectory = path / boost::filesystem::unique_path();
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
	auto adder2 = uow2->CreateFileAdder();
	CreateFile(filePath, "hell");
	CreateFile(deepFilePath, "hello");
	adder2->Add(path);
	uow2->Commit();

	// Assert
	const auto& added = adder2->GetAddedPaths();
	EXPECT_THAT(added, ::testing::Contains(path));
	EXPECT_THAT(added, ::testing::Contains(deepDirectory));
	EXPECT_THAT(added, ::testing::Contains(deepFilePath));
	EXPECT_THAT(added, ::testing::Contains(filePath));

	{
		auto uow3 = _forest->CreateUnitOfWork();
		auto finder3 = uow3->CreateFileFinder();
		EXPECT_TRUE(finder3->FindReference(path));
		EXPECT_TRUE(finder3->FindReference(deepDirectory));
		EXPECT_TRUE(finder3->FindReference(deepFilePath));
		EXPECT_TRUE(finder3->FindReference(filePath));

		// Deep file
		{
			const auto fileRef = finder3->FindReference(deepFilePath);
			EXPECT_TRUE(fileRef);
			const auto fileObject = finder3->GetObjectByAddress(fileRef->fileObjectAddress);
			EXPECT_EQ(helloBytes, uow3->GetBlob(fileObject.contentBlobAddress.value()));
		}

		// Updated file
		{
			const auto fileRef = finder3->FindReference(filePath);
			EXPECT_TRUE(fileRef);
			const auto fileObject = finder3->GetObjectByAddress(fileRef->fileObjectAddress);
			EXPECT_EQ(hellBytes, uow3->GetBlob(fileObject.contentBlobAddress.value()));
		}
	}
}


}
}
}
}
