#include "bslib/file/fs/path.hpp"

#include "bslib/unicode.hpp"

namespace af {
namespace bslib {
namespace file {
namespace fs {

NativePath NativeFromBoostPath(const boost::filesystem::path& path)
{
	auto result = WindowsPath(WideToUTF8String(path.wstring()));
	result.MakePreferred();
	return result;
}

boost::filesystem::path BoostPathFromNative(const NativePath& path)
{
	const auto wideString = UTF8ToWideString(path.ToExtendedString());
	return boost::filesystem::path(wideString);
}

}
}
}
}