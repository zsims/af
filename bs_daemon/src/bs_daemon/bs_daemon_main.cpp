#include "bs_daemon_lib/log.hpp"

#include <boost/log/utility/setup/common_attributes.hpp>

// Avoid size_t <-> int64 warnings on Windows x32
#pragma warning( push )
#pragma warning( disable : 4244 )
#include <server_http.hpp>
#pragma warning( pop )

#include <iostream>
#include <conio.h>

typedef SimpleWeb::Server<SimpleWeb::HTTP> HttpServer;

int main(int argc, char* argv[])
{
	boost::log::add_common_attributes();

	const int PORT = 8080;
	HttpServer server(PORT, 1);

	server.default_resource["GET"] = [&server](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		const std::string index("<html><head><title>AF Backup Service</title></head><body>Hi there</body></html>");
		*response << "HTTP/1.1 200 OK\r\nContent-Length:" << index.size() << "\r\n\r\n" << index;
	};

	std::thread server_thread([&server]() {
		BS_DAEMON_LOG_INFO << "Listening on " << server.config.address << ":" << server.config.port;
		server.start();
	});

	std::this_thread::sleep_for(std::chrono::seconds(1));

	std::cout << "Press any key to exit" << std::endl;
	_getch();
	std::cout << "Shutting down..." << std::endl;

	server.stop();
	server_thread.join();

	return 0;
}
