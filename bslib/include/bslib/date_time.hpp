#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>

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

}
}
