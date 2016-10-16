#include "test_util/TestBase.hpp"

#include "bslib/file/fs/operations.hpp"
#include "test_util/TestEnvironment.hpp"

#include <boost/filesystem.hpp>

#include <stdexcept>

namespace af {
namespace test_util {

TestBase::TestBase()
	: _testTemporaryPath(test_util::TestEnvironment::GetTemporaryDirectory() / boost::filesystem::unique_path())
	, _testBackup(_testTemporaryPath)
{
	boost::filesystem::create_directories(_testTemporaryPath);
}

TestBase::~TestBase()
{
	// Extended paths may have been used in tests. boost::filesystem::remove_all doesn't support extended paths in this case
	boost::system::error_code ec;
	bslib::file::fs::RemoveAll(bslib::file::fs::NativeFromBoostPath(_testTemporaryPath), ec);
}

boost::filesystem::path TestBase::GetUniqueTempPath() const
{
	return (_testTemporaryPath / boost::filesystem::unique_path());
}

bslib::file::fs::NativePath TestBase::GetUniqueExtendedTempPath() const
{
	// make a path > 260 characters
	auto result = bslib::file::fs::NativeFromBoostPath(GetUniqueTempPath()) / bslib::UTF8String(150, 'a') / bslib::UTF8String(150, 'b');
	bslib::file::fs::CreateDirectories(result);
	return result / boost::filesystem::unique_path().string();
}

bslib::blob::Address TestBase::CreateFile(const boost::filesystem::path& path, const bslib::UTF8String& content)
{
	return CreateFile(bslib::file::fs::NativeFromBoostPath(path), content);
}

bslib::blob::Address TestBase::CreateFile(const bslib::file::fs::NativePath& path, const bslib::UTF8String& content)
{
	const std::vector<uint8_t> binaryContent(content.begin(), content.end());
	auto f = bslib::file::fs::OpenFileWrite(path);
	if (!f)
	{
		throw std::runtime_error("Failed to create test file at " + path.ToString());
	}
	if (!binaryContent.empty())
	{
		f.write(reinterpret_cast<const char*>(&binaryContent[0]), binaryContent.size());
	}
	return bslib::blob::Address::CalculateFromContent(binaryContent);
}

}
}
