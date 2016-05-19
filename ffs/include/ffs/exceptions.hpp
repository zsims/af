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
	explicit DatabaseAlreadyExistsException(const std::string& path)
		: std::runtime_error("Database already exists at " + path)
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
	explicit DatabaseNotFoundException(const std::string& path)
		: std::runtime_error("Database not found at " + path)
	{
	}
};

class OpenDatabaseFailedException : public std::runtime_error
{
public:
	explicit OpenDatabaseFailedException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

}
}
