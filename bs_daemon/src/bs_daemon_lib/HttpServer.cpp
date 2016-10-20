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
	_simpleServer.default_resource["GET"] = [&](std::shared_ptr<SimpleServer::Response> response, std::shared_ptr<SimpleServer::Request> request) {
		const std::string index("<html><head><title>AF Backup Service</title></head><body>Hi there</body></html>");
		*response << "HTTP/1.1 200 OK\r\nContent-Length:" << index.size() << "\r\n\r\n" << index;
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
}

HttpServer::~HttpServer()
{
	Stop();
}

void HttpServer::Run()
{
	BS_DAEMON_LOG_INFO << "Starting HTTP server";
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
