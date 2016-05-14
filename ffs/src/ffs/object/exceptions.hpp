#pragma once

#include "ffs/Address.hpp"

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

class ObjectNotFoundException : public std::runtime_error
{
public:
	explicit ObjectNotFoundException(const ObjectAddress& address)
		: std::runtime_error(address.ToString())
	{
	}
};

}
}
}