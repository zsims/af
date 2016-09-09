#pragma once

#include "bslib/Address.hpp"

#include <stdexcept>
#include <string>

namespace af {
namespace bslib {
namespace file {

class EventNotFoundException : public std::runtime_error
{
public:
	explicit EventNotFoundException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

class AddFileEventFailedException : public std::runtime_error
{
public:
	explicit AddFileEventFailedException(const std::string& message)
		: std::runtime_error(message)
	{
	}

	explicit AddFileEventFailedException(int sqlError)
		: std::runtime_error("Failed to insert file event object, SQL error " + std::to_string(sqlError))
	{
	}
};

/**
* Given path couldn't be found.
*/
class PathNotFoundException : public std::runtime_error
{
public:
	explicit PathNotFoundException(const std::string& path)
		: std::runtime_error("Path not found at " + path)
	{
	}
};

/**
* Source path isn't of a supported type.
*/
class SourcePathNotSupportedException : public std::runtime_error
{
public:
	explicit SourcePathNotSupportedException(const std::string& path)
		: std::runtime_error("Object at path " + path + " is not of a supported type (directory/file)")
	{
	}
};

/**
* Target path isn't of a supported type.
*/
class TargetPathNotSupportedException : public std::runtime_error
{
public:
	explicit TargetPathNotSupportedException(const std::string& path)
		: std::runtime_error("Object at path " + path + " is not of a supported type (directory)")
	{
	}
};

}
}
}