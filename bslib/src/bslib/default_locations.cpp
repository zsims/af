#include "bslib/default_locations.hpp"

#include "bslib/unicode.hpp"

#include <boost/filesystem/path.hpp>

#ifdef WIN32
#include <shlobj.h>
#include <shlwapi.h>
#endif

#include <memory>
#include <string>

namespace af {
namespace bslib {

namespace {

boost::filesystem::path GetRoot()
{
	PWSTR buffer = nullptr;
	const auto result = SHGetKnownFolderPath(FOLDERID_ProgramData, 0, nullptr, &buffer);
	const std::unique_ptr<WCHAR, std::function<void(PWSTR)>> autoBuffer(buffer, [](PWSTR value) { CoTaskMemFree(value); });
	if (result != S_OK)
	{
		return boost::filesystem::path();
	}
	return boost::filesystem::path(autoBuffer.get()) / "af";
}

}

boost::filesystem::path GetDefaultBackupDatabasePath()
{
	return GetRoot() / "backup.db";
}

}
}