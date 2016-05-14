#pragma once

#include <stdexcept>

namespace af {
namespace ffs {

/**
 * A database at the given path already exists
 */
class DatabaseAlreadyExistsException : public std::runtime_error
{
public:
	explicit DatabaseAlreadyExistsException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

class CreateDatabaseFailedException : public std::runtime_error
{
public:
	explicit CreateDatabaseFailedException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

class DatabaseNotFoundException : public std::runtime_error
{
public:
	explicit DatabaseNotFoundException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

}
}
