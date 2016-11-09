#pragma once

#include "bslib/unicode.hpp"

#include <boost/operators.hpp>
#include <boost/uuid/uuid.hpp>

namespace af {
namespace bslib {

class Uuid
{
public:
	explicit Uuid(const UTF8String& value);
	UTF8String ToString() const;

	bool operator<(const Uuid& rhs) const;
	friend bool operator==(const Uuid& lhs, const Uuid& rhs);
	friend bool operator!=(const Uuid& lhs, const Uuid& rhs);

	static Uuid Create();
private:
	explicit Uuid(const boost::uuids::uuid& value);
	boost::uuids::uuid _value;
};

bool operator==(const Uuid& lhs, const Uuid& rhs);
bool operator!=(const Uuid& lhs, const Uuid& rhs);

std::ostream& operator<<(std::ostream& output, const Uuid& obj);

}
}
