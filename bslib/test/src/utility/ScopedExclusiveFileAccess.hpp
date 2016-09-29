#pragma once

#include "bslib/file/fs/path.hpp"

#include <boost/core/noncopyable.hpp>

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
	explicit ScopedExclusiveFileAccess(const file::fs::NativePath& path);
	~ScopedExclusiveFileAccess();
private:
	const file::fs::NativePath _path;
	/* HANDLE */void* _windowsHandle;
};

}
}
}
}
