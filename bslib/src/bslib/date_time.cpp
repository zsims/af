#include "bslib/date_time.hpp"

namespace af {
namespace bslib {

int32_t GetSecondsSinceEpoch(const boost::posix_time::ptime& ptime)
{
	static boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
	return (ptime - epoch).total_seconds();
}

boost::posix_time::ptime FromSecondsSinceEpoch(int32_t seconds)
{
	return boost::posix_time::from_time_t(static_cast<time_t>(seconds));
}

}
}
