#pragma once

#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/trivial.hpp>

#include <string>

namespace af {
namespace bslib {

typedef boost::log::sources::severity_channel_logger_mt<boost::log::trivial::severity_level, std::string> logger_type;

BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(bslib_logger, logger_type, (boost::log::keywords::channel = "bslib"))

}
}

#define BSLIB_LOG_TRACE BOOST_LOG_SEV(af::bslib::bslib_logger::get(), boost::log::trivial::trace)
#define BSLIB_LOG_DEBUG BOOST_LOG_SEV(af::bslib::bslib_logger::get(), boost::log::trivial::debug)
#define BSLIB_LOG_INFO BOOST_LOG_SEV(af::bslib::bslib_logger::get(), boost::log::trivial::info)
#define BSLIB_LOG_WARNING BOOST_LOG_SEV(af::bslib::bslib_logger::get(), boost::log::trivial::warning)
#define BSLIB_LOG_ERROR BOOST_LOG_SEV(af::bslib::bslib_logger::get(), boost::log::trivial::error)
#define BSLIB_LOG_FATAL BOOST_LOG_SEV(af::bslib::bslib_logger::get(), boost::log::trivial::fatal)
