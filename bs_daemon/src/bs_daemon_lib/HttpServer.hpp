#pragma once

#include "bslib/blob/BlobStoreManager.hpp"
#include "bs_daemon_lib/JobExecutor.hpp"

#include <boost/noncopyable.hpp>

// Avoid size_t <-> int64 warnings on Windows x32
#pragma warning( push )
#pragma warning( disable : 4244 )
#include <server_http.hpp>
#pragma warning( pop )

#include <thread>

typedef SimpleWeb::Server<SimpleWeb::HTTP> SimpleServer;

namespace af {
namespace bs_daemon {

/**
 * Exposes functionality over a HTTP API
 */
class HttpServer : public boost::noncopyable
{
public:
	HttpServer(int port, bslib::blob::BlobStoreManager& blobStoreManager, JobExecutor& jobExecutor);
	~HttpServer();
private:
	void Stop();
	void Run();

	bslib::blob::BlobStoreManager& _blobStoreManager;
	JobExecutor& _jobExecutor;
	SimpleServer _simpleServer;
	std::thread _serverThread;
};

}
}