#pragma once

#include "bslib/file/FileEvent.hpp"
#include "bslib/Uuid.hpp"

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
};

}
}
}