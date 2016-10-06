#include "TestBase.hpp"

#include "bslib/file/fs/operations.hpp"
#include "utility/TestEnvironment.hpp"

#include <boost/filesystem.hpp>

#include <stdexcept>

namespace af {
namespace bslib {
namespace test {

TestBase::TestBase()
	: _testTemporaryPath(utility::TestEnvironment::GetTemporaryDirectory() / boost::filesystem::unique_path())
	, _testBackup(_testTemporaryPath)
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

blob::Address TestBase::CreateFile(const boost::filesystem::path& path, const UTF8String& content)
{
	return CreateFile(file::fs::NativeFromBoostPath(path), content);
}

blob::Address TestBase::CreateFile(const file::fs::NativePath& path, const UTF8String& content)
{
	const std::vector<uint8_t> binaryContent(content.begin(), content.end());
	auto f = file::fs::OpenFileWrite(path);
	if (!f)
	{
		throw std::runtime_error("Failed to create test file at " + path.ToString());
	}
	if (!binaryContent.empty())
	{
		f.write(reinterpret_cast<const char*>(&binaryContent[0]), binaryContent.size());
	}
	return blob::Address::CalculateFromContent(binaryContent);
}

}
}
}
