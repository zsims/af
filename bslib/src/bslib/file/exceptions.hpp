#pragma once

#include "bslib/Address.hpp"

#include <stdexcept>
#include <string>

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

class RefNotFoundException : public std::runtime_error
{
public:
	explicit RefNotFoundException(const std::string& fullPath)
		: std::runtime_error("Reference with the full path " + fullPath + " could not be found")
	{
	}

	RefNotFoundException(const std::string& fullPath, int sqlError)
		: std::runtime_error("Reference with the full path " + fullPath + " could not be found, SQL error " + std::to_string(sqlError))
	{
	}
};

class AddRefFailedException : public std::runtime_error
{
public:
	AddRefFailedException(const std::string& fullPath, int sqlError)
		: std::runtime_error("Failed to insert reference, SQL error " + std::to_string(sqlError))
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