#include "bslib/date_time.hpp"

#include <boost/algorithm/string/trim.hpp>

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

std::string ToIso8601Utc(const boost::posix_time::ptime& ptime)
{
	return boost::posix_time::to_iso_extended_string(ptime) + "Z";
}

boost::posix_time::ptime FromIso8601Utc(const std::string& rawDate)
{
	// remove the zone if any, as parse_delimited_time doesn't handle this
	const auto trimmed = boost::trim_right_copy_if(rawDate, isalpha);
	return boost::date_time::parse_delimited_time<boost::posix_time::ptime>(trimmed, 'T');
}

}
}
