#pragma once

#include "bslib/file/fs/path.hpp"

#include <boost/core/noncopyable.hpp>

namespace af {
namespace bslib {
namespace file {
namespace test_utility {

/**
 * Changes the working directory for the duration of this object
 */
class ScopedWorkingDirectory : private boost::noncopyable
{
public:
	/**
	 * Set the working directory to the given path
	 * \remarks Paths longer than MAX_PATH are not supported on Windows due to limitations with SetCurrentDirectoryW()
	 */
	explicit ScopedWorkingDirectory(const fs::NativePath& path);
	~ScopedWorkingDirectory();
private:
	fs::NativePath _originalPath;
};

}
}
}
}
