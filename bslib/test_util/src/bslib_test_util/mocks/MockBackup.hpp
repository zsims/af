#include "bslib/Backup.hpp"
#include "bslib/blob/BlobStore.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace af {
namespace bslib_test_util {
namespace mocks {

class MockBackup : public bslib::Backup
{
public:
	MockBackup() 
		: bslib::Backup("Not Real", "Testing")
	{

	}
	virtual ~MockBackup() { }

	// Support mocking unique_ptr parameters per https://github.com/google/googletest/blob/master/googlemock/docs/CookBook.md#mocking-methods-that-use-move-only-types
	virtual void AddBlobStore(std::unique_ptr<bslib::blob::BlobStore> blobStore) override
	{
		return AddBlobStoreProxy(blobStore.get());
	}

	MOCK_METHOD0(CreateUnitOfWork, std::unique_ptr<bslib::UnitOfWork>());
	MOCK_METHOD1(AddBlobStoreProxy, void(bslib::blob::BlobStore* blobStore));
	MOCK_METHOD0(Open, void());
	MOCK_METHOD0(Create, void());
	MOCK_METHOD0(OpenOrCreate, void());
	MOCK_METHOD0(SaveDatabaseCopy, void());
	MOCK_METHOD0(GetBackupDatabase, bslib::BackupDatabase&());
};

}
}
}
