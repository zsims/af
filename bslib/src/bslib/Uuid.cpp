#include "bslib/Uuid.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <sstream>
#include <mutex>

namespace af {
namespace bslib {

Uuid::Uuid(const boost::uuids::uuid& value)
	: _value(value)
{
}

Uuid::Uuid(const UTF8String& value)
{
	std::istringstream iss(value);
	iss >> _value;
}

UTF8String Uuid::ToString() const
{
	return boost::uuids::to_string(_value);
}

Uuid Uuid::Create()
{
	// According to http://stackoverflow.com/questions/18890355/is-boostuuidsrandom-generator-thread-safe, the generator is not thread safe
	static std::mutex generatorMutex;
	static boost::uuids::random_generator generator;

	std::unique_lock<std::mutex> lock;
	return Uuid(generator());
}

bool Uuid::operator<(const Uuid& rhs) const
{
	return _value < rhs._value;
}

std::ostream& operator<<(std::ostream& output, const Uuid& value)
{
	return output << value.ToString();
}

bool operator==(const Uuid& lhs, const Uuid& rhs)
{
	return lhs._value == rhs._value;
}

bool operator!=(const Uuid& lhs, const Uuid& rhs)
{
	return lhs._value != rhs._value;
}

}
}