#pragma once

#include "bslib/UnitOfWork.hpp"

namespace af {
namespace bs_daemon {

class Job
{
public:
	virtual ~Job() { }
	/**
	 * Run the job. 
	 * \remarks The unit of work must be explicitly committed by the job
	 */
	virtual void Run(bslib::UnitOfWork& unitOfWork) = 0;
};

}
}
