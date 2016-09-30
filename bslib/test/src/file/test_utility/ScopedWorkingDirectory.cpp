#include "file/test_utility/ScopedWorkingDirectory.hpp"

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace af {
namespace bslib {
namespace file {
namespace test_utility {

ScopedWorkingDirectory::ScopedWorkingDirectory(const fs::NativePath& path)
	: _originalPath(WideToUTF8String(boost::filesystem::current_path().wstring()))
{
	boost::filesystem::current_path(UTF8ToWideString(path.ToExtendedString()));
}

ScopedWorkingDirectory::~ScopedWorkingDirectory()
{
	boost::system::error_code ec;
	boost::filesystem::current_path(UTF8ToWideString(_originalPath.ToExtendedString()), ec);
}

}
}
}
}
