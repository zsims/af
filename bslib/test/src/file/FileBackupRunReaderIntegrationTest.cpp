#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileBackupRunReader.hpp"
#include "bslib_test_util/TestBase.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <functional>
#include <memory>

namespace af {
namespace bslib {
namespace file {
namespace test {

class FileBackupRunReaderIntegrationTest : public bslib_test_util::TestBase
{
protected:
	FileBackupRunReaderIntegrationTest()
	{
		_testBackup.Create();
		_uow = _testBackup.GetBackup().CreateUnitOfWork();
	}

	std::unique_ptr<UnitOfWork> _uow;
};

}
}
}
}
