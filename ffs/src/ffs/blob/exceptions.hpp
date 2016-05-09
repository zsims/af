#pragma once

#include <stdexcept>

namespace af {
namespace ffs {
namespace blob {

class DuplicateBlobException : public std::runtime_error
{
public:
	explicit DuplicateBlobException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

}
}
}