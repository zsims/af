#pragma once

#include <stdexcept>
#include <string>

namespace af {
namespace bslib {

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
	CreateDatabaseFailedException(const std::string& path, int sqliteError)
		: std::runtime_error("Cannot create database at " + path + ". SQLite returned " + std::to_string(sqliteError))
	{
	}

	CreateDatabaseFailedException(const std::string& path, int sqliteError, const std::string& sqliteErrorMessage)
		: std::runtime_error("Cannot create database at " + path + ". SQLite returned " + std::to_string(sqliteError))
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

class SaveDatabaseAsFailedException : public std::runtime_error
{
public:
	explicit SaveDatabaseAsFailedException(const std::string& path)
		: std::runtime_error("Database not found at " + path)
	{
	}
};

}
}
