#include "bslib/Backup.hpp"
#include "bs_daemon_lib/log.hpp"
#include "bs_daemon_lib/HttpServer.hpp"

#include <boost/filesystem.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <iostream>
#include <conio.h>

namespace {

int Run()
{
	const auto defaultDbPath = af::bslib::GetDefaultBackupDatabasePath();
	if (defaultDbPath.empty())
	{
		BS_DAEMON_LOG_FATAL << "Failed to determine the location of the backup database";
		return -1;
	}
	BS_DAEMON_LOG_INFO << "Using the backup database from " << defaultDbPath << std::endl;
	boost::filesystem::create_directories(defaultDbPath.parent_path());

	af::bslib::Backup backup(defaultDbPath, "CLI");
	backup.OpenOrCreate();

	af::bs_daemon::HttpServer server(8080);

	std::cout << "Press any key to exit" << std::endl;
	_getch();
	std::cout << "Shutting down..." << std::endl;
	return 0;
}

}

int main(int argc, char* argv[])
{
	boost::log::add_common_attributes();

	try
	{
		return Run();
	}
	catch (const std::exception& e)
	{
		BS_DAEMON_LOG_FATAL << "Unhandled exception: " << e.what();
		return 1;
	}
	catch (...)
	{
		BS_DAEMON_LOG_FATAL << "Unhandled exception";
		return -1;
	}
}
