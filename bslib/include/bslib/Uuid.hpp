#pragma once

#include "bslib/unicode.hpp"

#include <boost/uuid/uuid.hpp>

#include <array>
#include <stdexcept>

namespace af {
namespace bslib {

class UuidInvalidException : public std::runtime_error
{
public:
	explicit UuidInvalidException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

class Uuid
{
public:
	typedef std::array<uint8_t, 16> BinaryType;
	explicit Uuid(const UTF8String& value);
	explicit Uuid(const void* rawBuffer, int bufferLength);
	UTF8String ToString() const;
	UTF8String ToDashlessString() const;
	BinaryType ToArray() const;

	bool operator<(const Uuid& rhs) const;
	friend bool operator==(const Uuid& lhs, const Uuid& rhs);
	friend bool operator!=(const Uuid& lhs, const Uuid& rhs);

	static Uuid Create();
	static const Uuid Empty;
private:
	explicit Uuid(const boost::uuids::uuid& value);
	boost::uuids::uuid _value;
};

bool operator==(const Uuid& lhs, const Uuid& rhs);
bool operator!=(const Uuid& lhs, const Uuid& rhs);

std::ostream& operator<<(std::ostream& output, const Uuid& obj);

}
}
