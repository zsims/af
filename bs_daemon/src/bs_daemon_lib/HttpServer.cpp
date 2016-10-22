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

typedef std::function<void(std::shared_ptr<SimpleServer::Response>, std::shared_ptr<SimpleServer::Request>)> RequestHandler;

namespace {

struct HttpJsonRequest
{
	boost::property_tree::ptree content;
};

struct HttpJsonResponse
{
	HttpJsonResponse(int statusCode, const std::string& statusDescription)
		: statusDescription(statusDescription)
		, statusCode(statusCode)
	{
	}

	HttpJsonResponse(int statusCode, const std::string& statusDescription, const std::string& errorMessage)
		: statusDescription(statusDescription)
		, statusCode(statusCode)
	{
		content.push_back(boost::property_tree::ptree::value_type("error", errorMessage));
	}

	HttpJsonResponse(int statusCode, const std::string& statusDescription, const boost::property_tree::ptree& content)
		: statusDescription(statusDescription)
		, statusCode(statusCode)
		, content(content)
	{
	}
	std::string statusDescription;
	int statusCode;
	boost::property_tree::ptree content;
};

typedef std::function<HttpJsonResponse(const HttpJsonRequest&)> JsonRequestHandler;


void SendJsonResponse(const HttpJsonResponse& jsonResponse, std::shared_ptr<SimpleServer::Response> response)
{
	std::string content;
	if (!jsonResponse.content.empty())
	{
		std::stringstream ss;
		boost::property_tree::write_json(ss, jsonResponse.content);
		content = ss.str();
	}
	*response << "HTTP/1.1 " << jsonResponse.statusCode << " " << jsonResponse.statusDescription << "\r\n";
	if (!content.empty())
	{
		*response << "Content-Length: " << content.length() - 1 << "\r\n";
		*response << "Content-Type: application/json; charset=utf-8\r\n";
	}
	*response << "\r\n";
	*response << content;
}

/**
 * Returns a request handler from the given JsonRequest -> JsonResponse handler that will correctly handle JSON requests
*/
RequestHandler JsonHandler(JsonRequestHandler handler)
{
	return [=](std::shared_ptr<SimpleServer::Response> response, std::shared_ptr<SimpleServer::Request> request) {
		try
		{
			// TODO: Check JSON content type
			HttpJsonRequest jsonRequest;
			boost::property_tree::read_json(request->content, jsonRequest.content);
			const auto& jsonResponse = handler(jsonRequest);
			SendJsonResponse(jsonResponse, response);
		}
		catch (const boost::property_tree::json_parser_error& e)
		{
			const HttpJsonResponse error(400, "Bad Request", "Invalid JSON: " + e.message());
			SendJsonResponse(error, response);
		}
		catch (std::exception& e)
		{
			const HttpJsonResponse error(500, "Internal Server Error", std::string(e.what()));
			SendJsonResponse(error, response);
		}
	};
}

}

HttpServer::HttpServer(int port, JobExecutor& jobExecutor)
	: _jobExecutor(jobExecutor)
	, _simpleServer(port, /* number of threads = */ 1)
	, _serverThread(&HttpServer::Run, this)
{
	_simpleServer.config.address = "127.0.0.1";
	_simpleServer.config.reuse_address = true;

	_simpleServer.default_resource["GET"] = [&](std::shared_ptr<SimpleServer::Response> response, std::shared_ptr<SimpleServer::Request> request) {
		*response << "HTTP/1.1 404 Not Found\r\n\r\n";
	};

	_simpleServer.resource["^/file$"]["POST"] = JsonHandler([&](const HttpJsonRequest& request) {
		const auto& path = request.content.get_optional<std::string>("path");
		if (!path)
		{
			return HttpJsonResponse(400, "Bad Request", "path is required");
		}
		_jobExecutor.Queue(std::make_unique<FileBackupJob>(path.get()));
		return HttpJsonResponse(202, "Accepted");
	});

	// Test API that takes JSON and deserializes it, then sends it back as JSON
	_simpleServer.resource["^/ping$"]["POST"] = JsonHandler([](const HttpJsonRequest& request) {
		return HttpJsonResponse(200, "OK", request.content);
	});
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
