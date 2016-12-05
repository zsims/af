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

	// optional ancestor path id, matches all events that have an ancestor path with this id
	boost::optional<int64_t> ancestorPathId;

	// optional distance to the ancestor path, e.g. 1 = must be a parent
	boost::optional<unsigned> ancestorPathDistance;

	// optional how deep the paths must be
	boost::optional<unsigned> pathDepth;
};

}
}
}