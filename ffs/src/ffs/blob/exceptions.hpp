#pragma once

#include "ffs/Address.hpp"

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

class BlobReadException: public std::runtime_error
{
public:
	explicit BlobReadException(const BlobAddress& address)
		: std::runtime_error(address.ToString())
	{
	}
};

class AddBlobFailedException : public std::runtime_error
{
public:
	explicit AddBlobFailedException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

class GetBlobsFailedException : public std::runtime_error
{
public:
	explicit GetBlobsFailedException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

}
}
}