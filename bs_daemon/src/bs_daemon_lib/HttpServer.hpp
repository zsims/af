#pragma once

#include <boost/noncopyable.hpp>

// Avoid size_t <-> int64 warnings on Windows x32
#pragma warning( push )
#pragma warning( disable : 4244 )
#include <server_http.hpp>
#pragma warning( pop )

#include <thread>

namespace af {
namespace bs_daemon {

/**
 * Exposes functionality over a HTTP API
 */
class HttpServer : public boost::noncopyable
{
public:
	HttpServer(int port);
	~HttpServer();
	void Stop();
private:
	typedef SimpleWeb::Server<SimpleWeb::HTTP> SimpleServer;
	void Run();

	SimpleServer _simpleServer;
	std::thread _serverThread;
};

}
}
