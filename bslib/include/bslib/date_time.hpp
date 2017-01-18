#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>

namespace af {
namespace bslib {

/**
 * Gets the number of seconds since the UNIX epoch
 */
int32_t GetSecondsSinceEpoch(const boost::posix_time::ptime& ptime);

/**
 * Returns a ptime from the total number of seconds since the UNIX epoch
 */
boost::posix_time::ptime FromSecondsSinceEpoch(int32_t seconds);

/**
 * Converts the given posix time/date into an ISO 8601 date with a UTC qualifier, e.g. 2016-11-20T09:49:30Z
 */
std::string ToIso8601Utc(const boost::posix_time::ptime& ptime);

/**
 * Converts the given ISO 8601 date and time to a posix time/date
 */
boost::posix_time::ptime FromIso8601Utc(const std::string& rawDate);

}
}
