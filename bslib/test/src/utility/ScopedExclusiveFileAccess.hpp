#pragma once

#include <boost/core/noncopyable.hpp>
#include <boost/filesystem/path.hpp>

namespace af {
namespace bslib {
namespace test {
namespace utility {

/**
 * Access the given file exclusively so it can't be opened elsewhere
 */
class ScopedExclusiveFileAccess : private boost::noncopyable
{
public:
	explicit ScopedExclusiveFileAccess(const boost::filesystem::path& path);
	~ScopedExclusiveFileAccess();
private:
	const boost::filesystem::path _path;
	/* HANDLE */void* _windowsHandle;
};

}
}
}
}
