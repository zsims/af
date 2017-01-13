#pragma once

#include "bslib/file/FileEvent.hpp"
#include "bslib/Uuid.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <set>

namespace af {
namespace bslib {
namespace file {

struct FileEventSearchCriteria
{
	// optional run id to match
	boost::optional<Uuid> runId;

	// optional actions to match
	std::set<FileEventAction> actions;

	// events raised before and including the given date/time
	boost::optional<boost::posix_time::ptime> before;
};

}
}
}