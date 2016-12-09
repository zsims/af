#pragma once

#include <ostream>
#include <string>

namespace af {
namespace bslib {
namespace file {

enum class FileType : int
{
	RegularFile = 0,
	Directory,
	Unsupported
};

std::string ToString(FileType type);

inline std::ostream& operator<<(std::ostream & os, FileType type)
{
	return os << ToString(type);
}

}
}
}
