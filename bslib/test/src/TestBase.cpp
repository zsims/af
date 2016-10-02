#include "TestBase.hpp"

#include "bslib/file/fs/operations.hpp"
#include "utility/TestEnvironment.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace af {
namespace bslib {
namespace test {

std::string GenerateUuid()
{
	boost::uuids::random_generator generator;
	boost::uuids::uuid uuid = generator();
	return boost::uuids::to_string(uuid);
}

TestBase::TestBase()
	: _testTemporaryPath(utility::TestEnvironment::GetTemporaryDirectory() / GenerateUuid())
{
	file::fs::CreateDirectories(_testTemporaryPath);
}

TestBase::~TestBase()
{
	boost::system::error_code ec;
	file::fs::RemoveAll(_testTemporaryPath, ec);
}

file::fs::NativePath TestBase::GetUniqueTempPath() const
{
	return (_testTemporaryPath / GenerateUuid());
}

file::fs::NativePath TestBase::GetUniqueExtendedTempPath() const
{
	// make a path > 260 characters
	auto intermediate = GetUniqueTempPath() / UTF8String(150, 'a') / UTF8String(150, 'b');
	file::fs::CreateDirectories(intermediate);
	return (intermediate / GenerateUuid());
}

}
}
}
