#pragma once

#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/trivial.hpp>

#include <string>

namespace af {
namespace bs_daemon {

typedef boost::log::sources::severity_channel_logger_mt<boost::log::trivial::severity_level, std::string> logger_type;

BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(bs_daemon_logger, logger_type, (boost::log::keywords::channel = "bs_daemon"))

}
}

#define BS_DAEMON_LOG_TRACE BOOST_LOG_SEV(af::bs_daemon::bs_daemon_logger::get(), boost::log::trivial::trace)
#define BS_DAEMON_LOG_DEBUG BOOST_LOG_SEV(af::bs_daemon::bs_daemon_logger::get(), boost::log::trivial::debug)
#define BS_DAEMON_LOG_INFO BOOST_LOG_SEV(af::bs_daemon::bs_daemon_logger::get(), boost::log::trivial::info)
#define BS_DAEMON_LOG_WARNING BOOST_LOG_SEV(af::bs_daemon::bs_daemon_logger::get(), boost::log::trivial::warning)
#define BS_DAEMON_LOG_ERROR BOOST_LOG_SEV(af::bs_daemon::bs_daemon_logger::get(), boost::log::trivial::error)
#define BS_DAEMON_LOG_FATAL BOOST_LOG_SEV(af::bs_daemon::bs_daemon_logger::get(), boost::log::trivial::fatal)
