#include "utility/TestEnvironment.hpp"

#include <boost/filesystem.hpp>

namespace af {
namespace bslib {
namespace test {
namespace utility {

boost::filesystem::path TestEnvironment::_temporaryDirectory;

void TestEnvironment::SetTemporaryDirectory(const boost::filesystem::path& path)
{
	_temporaryDirectory = path;
}

boost::filesystem::path TestEnvironment::GetTemporaryDirectory()
{
	return _temporaryDirectory;
}

void TestEnvironment::SetUp()
{
	boost::filesystem::create_directories(_temporaryDirectory);
}

void TestEnvironment::TearDown()
{
	boost::system::error_code ec;
	boost::filesystem::remove_all(_temporaryDirectory, ec);
}

}
}
}
}
