#pragma once

#include <string>

namespace af {
namespace cfs {

/**
 * Represents a single content based file system
 */
class FileSystem
{
public:
	FileSystem(const std::string& treePath);
	virtual ~FileSystem();

private:
	const std::string _treePath;
};

}
}