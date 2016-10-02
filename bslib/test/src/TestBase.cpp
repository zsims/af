#include "TestBase.hpp"

#include "bslib/file/fs/operations.hpp"
#include "utility/TestEnvironment.hpp"

#include <boost/filesystem.hpp>

namespace af {
namespace bslib {
namespace test {

TestBase::TestBase()
	: _testTemporaryPath(utility::TestEnvironment::GetTemporaryDirectory() / boost::filesystem::unique_path())
	, _testForest(_testTemporaryPath)
{
	boost::filesystem::create_directories(_testTemporaryPath);
}

TestBase::~TestBase()
{
	// Extended paths may have been used in tests. boost::filesystem::remove_all doesn't support extended paths in this case
	boost::system::error_code ec;
	file::fs::RemoveAll(file::fs::NativeFromBoostPath(_testTemporaryPath), ec);
}

boost::filesystem::path TestBase::GetUniqueTempPath() const
{
	return (_testTemporaryPath / boost::filesystem::unique_path());
}

file::fs::NativePath TestBase::GetUniqueExtendedTempPath() const
{
	// make a path > 260 characters
	auto result = file::fs::NativeFromBoostPath(GetUniqueTempPath()) / UTF8String(150, 'a') / UTF8String(150, 'b');
	file::fs::CreateDirectories(result);
	return result / boost::filesystem::unique_path().string();
}

}
}
}
