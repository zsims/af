#pragma once

#include "bslib/Address.hpp"

#include <stdexcept>

namespace af {
namespace bslib {
namespace file {

class ObjectNotFoundException : public std::runtime_error
{
public:
	explicit ObjectNotFoundException(const std::string& message)
		: std::runtime_error(message)
	{
	}

	explicit ObjectNotFoundException(const ObjectAddress& address)
		: std::runtime_error("Object with the address " + address.ToString() + " could not be found")
	{
	}
};

class AddObjectFailedException : public std::runtime_error
{
public:
	explicit AddObjectFailedException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

}
}
}