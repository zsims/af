#include "bslib/forest.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/file/FileObjectInfoRepository.hpp"
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

class FileAdderIntegrationTest : public testing::Test
{
protected:
	FileAdderIntegrationTest()
		: _forestDbPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.fdb"))
	{
		auto blobStore = std::make_unique<blob::NullBlobStore>();
		_forest.reset(new Forest(_forestDbPath.string(), std::move(blobStore)));
		_forest->Create();
	}

	~FileAdderIntegrationTest()
	{
		_forest.reset();

		boost::system::error_code ec;
		boost::filesystem::remove(_forestDbPath, ec);
	}

	const boost::filesystem::path _forestDbPath;
	std::unique_ptr<Forest> _forest;
};

TEST_F(FileAdderIntegrationTest, AddFile)
{
	// Arrange
	auto uow = _forest->CreateUnitOfWork();
	auto adder = uow->CreateFileAdder();

	// Act
	const auto address = adder->Add("/here", { 1, 2, 3 });
	uow->Commit();

	// Assert
	{
		auto uow2 = _forest->CreateUnitOfWork();
		EXPECT_NO_THROW(uow2->GetFileObject(address));
	}
}

}
}
}
}
