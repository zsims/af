#include "bslib/UnitOfWork.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace af {
namespace bslib_test_util {
namespace mocks {

class MockUnitOfWork : public bslib::UnitOfWork
{
public:
	virtual ~MockUnitOfWork() { }
	MOCK_METHOD0(Commit, void());
	MOCK_METHOD0(CreateFileAdder, std::unique_ptr<bslib::file::FileAdder>());
	MOCK_METHOD0(CreateFileRestorer, std::unique_ptr<bslib::file::FileRestorer>());
	MOCK_METHOD0(CreateFileFinder, std::unique_ptr<bslib::file::FileFinder>());
	MOCK_CONST_METHOD1(GetBlob, std::vector<uint8_t>(const bslib::blob::Address& address));
};

}
}
}
