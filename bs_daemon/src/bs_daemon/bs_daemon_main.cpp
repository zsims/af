
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
	const int PORT = 8080;
	HttpServer server(PORT, 1);

	server.default_resource["GET"] = [&server](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		const std::string index("<html><head><title>AF Backup Service</title></head><body>Hi there</body></html>");
		*response << "HTTP/1.1 200 OK\r\nContent-Length:" << index.size() << "\r\n\r\n" << index;
	};

	std::thread server_thread([&server]() {
		server.start();
	});

	std::this_thread::sleep_for(std::chrono::seconds(1));

	std::cout << "Listening on localhost:" << PORT << std::endl << std::endl;
	std::cout << "Press any key to exit" << std::endl;
	_getch();
	std::cout << "Shutting down..." << std::endl;

	server.stop();
	server_thread.join();

	return 0;
}
