#include "bslib/Backup.hpp"
#include "bslib/blob/BlobStore.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace af {
namespace test_util {
namespace mocks {

class MockBackup : public bslib::Backup
{
public:
	virtual ~MockBackup() { }
	MOCK_METHOD0(CreateUnitOfWork, std::unique_ptr<bslib::UnitOfWork>());
	MOCK_METHOD1(AddBlobStore, void(std::unique_ptr<bslib::blob::BlobStore> blobStore));
	MOCK_METHOD0(Open, void());
	MOCK_METHOD0(Create, void());
	MOCK_METHOD0(OpenOrCreate, void());
	MOCK_METHOD0(GetBackupDatabase, bslib::BackupDatabase&());
};

}
}
}
