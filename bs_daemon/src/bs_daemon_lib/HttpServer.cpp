#include "bs_daemon_lib/HttpServer.hpp"

#include "bs_daemon_lib/log.hpp"

namespace af {
namespace bs_daemon {

HttpServer::HttpServer(int port)
	: _simpleServer(port, /* number of threads = */ 1)
	, _serverThread(&HttpServer::Run, this)
{
	_simpleServer.default_resource["GET"] = [&](std::shared_ptr<SimpleServer::Response> response, std::shared_ptr<SimpleServer::Request> request) {
		const std::string index("<html><head><title>AF Backup Service</title></head><body>Hi there</body></html>");
		*response << "HTTP/1.1 200 OK\r\nContent-Length:" << index.size() << "\r\n\r\n" << index;
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
