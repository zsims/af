#include "bslib/Backup.hpp"
#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobStoreManager.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace af {
namespace bslib_test_util {
namespace mocks {

class MockBackup : public bslib::Backup
{
public:
	MockBackup(const bslib::blob::BlobStoreManager& blobStoreManager) 
		: bslib::Backup("Not Real", "Testing", blobStoreManager)
	{

	}
	virtual ~MockBackup() { }

	MOCK_METHOD0(CreateUnitOfWork, std::unique_ptr<bslib::UnitOfWork>());
	MOCK_METHOD0(Open, void());
	MOCK_METHOD0(Create, void());
	MOCK_METHOD0(OpenOrCreate, void());
	MOCK_METHOD0(SaveDatabaseCopy, void());
	MOCK_METHOD0(GetBackupDatabase, bslib::BackupDatabase&());
};

}
}
}
