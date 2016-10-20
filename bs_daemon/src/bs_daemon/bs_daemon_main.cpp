#include "bs_daemon_lib/log.hpp"
#include "bs_daemon_lib/HttpServer.hpp"

#include <boost/log/utility/setup/common_attributes.hpp>

#include <iostream>
#include <conio.h>

int main(int argc, char* argv[])
{
	boost::log::add_common_attributes();
	af::bs_daemon::HttpServer server(8080);

	std::cout << "Press any key to exit" << std::endl;
	_getch();
	std::cout << "Shutting down..." << std::endl;

	return 0;
}
