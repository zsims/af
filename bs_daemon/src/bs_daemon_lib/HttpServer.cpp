#include "bs_daemon_lib/HttpServer.hpp"

#include "bs_daemon_lib/FileBackupJob.hpp"
#include "bs_daemon_lib/log.hpp"

// Work around https://svn.boost.org/trac/boost/ticket/11599
#pragma warning( push )
#pragma warning( disable : 4715)
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#pragma warning( pop )

#include <memory>

namespace af {
namespace bs_daemon {

HttpServer::HttpServer(int port, JobExecutor& jobExecutor)
	: _jobExecutor(jobExecutor)
	, _simpleServer(port, /* number of threads = */ 1)
	, _serverThread(&HttpServer::Run, this)
{
	_simpleServer.config.address = "127.0.0.1";

	_simpleServer.default_resource["GET"] = [&](std::shared_ptr<SimpleServer::Response> response, std::shared_ptr<SimpleServer::Request> request) {
		*response << "HTTP/1.1 404 Not Found\r\n\r\n";
	};

	_simpleServer.resource["^/file$"]["POST"] = [&](std::shared_ptr<SimpleServer::Response> response, std::shared_ptr<SimpleServer::Request> request) {
		try {
			boost::property_tree::ptree pt;
			boost::property_tree::read_json(request->content, pt);

			auto path = pt.get<std::string>("path");

			// kick off the job
			_jobExecutor.Queue(std::make_unique<FileBackupJob>(path));

			*response << "HTTP/1.1 204 No Content\r\n\r\n";
		}
		catch (std::exception& e) {
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};

	// Test API that takes JSON and deserializes it, then sends it back as JSON
	_simpleServer.resource["^/ping$"]["POST"] = [&](std::shared_ptr<SimpleServer::Response> response, std::shared_ptr<SimpleServer::Request> request) {
		try {
			// Note this should support UTF-8, according to the property tree docs: https://svn.boost.org/trac/boost/ticket/8883
			boost::property_tree::ptree pt;
			boost::property_tree::read_json(request->content, pt);
			std::stringstream ss;
			boost::property_tree::write_json(ss, pt);
			const auto content = ss.str();
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << content.length() << "\r\nContent-Type: application/json; charset=utf-8\r\n\r\n" << content;
		}
		catch (std::exception& e) {
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
		}
	};
}

HttpServer::~HttpServer()
{
	Stop();
}

void HttpServer::Run()
{
	BS_DAEMON_LOG_INFO << "Starting HTTP server on " << _simpleServer.config.address << ":" << _simpleServer.config.port;
	// blocks until the server stops
	_simpleServer.start();
}

void HttpServer::Stop()
{
	BS_DAEMON_LOG_INFO << "Stopping HTTP server";
	if (_serverThread.joinable())
	{
		_simpleServer.stop();
		_serverThread.join();
	}
}

}
}
