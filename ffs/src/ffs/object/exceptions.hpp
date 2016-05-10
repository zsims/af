#pragma once

#include <stdexcept>

namespace af {
namespace ffs {
namespace object {

class DuplicateObjectException : public std::runtime_error
{
public:
	explicit DuplicateObjectException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

}
}
}