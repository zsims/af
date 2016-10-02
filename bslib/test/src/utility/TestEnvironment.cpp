#include "utility/TestEnvironment.hpp"

#include "bslib/file/fs/operations.hpp"

namespace af {
namespace bslib {
namespace test {
namespace utility {

file::fs::NativePath TestEnvironment::_temporaryDirectory;

void TestEnvironment::SetTemporaryDirectory(const file::fs::NativePath& path)
{
	_temporaryDirectory = path;
}

file::fs::NativePath TestEnvironment::GetTemporaryDirectory()
{
	return _temporaryDirectory;
}

void TestEnvironment::SetUp()
{
	file::fs::CreateDirectories(_temporaryDirectory);
}

void TestEnvironment::TearDown()
{
	boost::system::error_code ec;
	file::fs::RemoveAll(_temporaryDirectory, ec);
}

}
}
}
}
