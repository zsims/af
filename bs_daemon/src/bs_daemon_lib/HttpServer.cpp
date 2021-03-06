#include "bs_daemon_lib/HttpServer.hpp"

#include "bs_daemon_lib/FileBackupJob.hpp"
#include "bs_daemon_lib/log.hpp"
#include "bslib/date_time.hpp"
#include "bslib/file/FileBackupRunSearchCriteria.hpp"
#include "bslib/file/FileEventSearchCriteria.hpp"

#include <boost/algorithm/string/split.hpp>
#include <boost/tokenizer.hpp>

#include <json.hpp>
#include <network/uri.hpp>

#include <memory>

namespace af {
namespace bs_daemon {

typedef std::function<void(std::shared_ptr<SimpleServer::Response>, std::shared_ptr<SimpleServer::Request>)> RequestHandler;

namespace {

const std::string ALLOWED_CORS_ORIGIN = "http://localhost:3000";

struct HttpJsonRequest
{
	HttpJsonRequest(const SimpleServer::Request& originalRequest, const network::uri& requestUri)
		: originalRequest(originalRequest)
		, uri(requestUri)
	{
	}

	/**
	 * Returns query string parameters, duplicate keys are resolved non-deterministically 
	 */
	const std::map<bslib::UTF8String, bslib::UTF8String> GetQueryParameters() const
	{
		std::map<bslib::UTF8String, bslib::UTF8String> result;

		// find key=value separated by '&'
		const auto queryString = uri.query().to_string();
		for (
			auto it = boost::split_iterator<std::string::const_iterator>(queryString, boost::first_finder("&", boost::is_equal()));
			it != boost::split_iterator<std::string::const_iterator>();
			++it)
		{
			const auto equalIt = std::find(it->begin(), it->end(), '=');
			std::string decodedKey;
			network::uri::decode(it->begin(), equalIt, std::back_inserter(decodedKey));

			if (equalIt != it->end())
			{
				std::string decodedValue;
				network::uri::decode(equalIt + 1, it->end(), std::back_inserter(decodedValue));
				result.insert(std::make_pair(decodedKey, decodedValue));
			}
			else
			{
				result.insert(std::make_pair(decodedKey, ""));
			}
		}

		return result;
	}

	const SimpleServer::Request& originalRequest;
	const network::uri uri;
	nlohmann::json content;
};

struct HttpJsonResponse
{
	HttpJsonResponse(int statusCode, const std::string& statusDescription)
		: statusDescription(statusDescription)
		, statusCode(statusCode)
	{
	}

	HttpJsonResponse(int statusCode, const std::string& statusDescription, const nlohmann::json& content)
		: statusDescription(statusDescription)
		, statusCode(statusCode)
		, content(content)
	{
	}

	static HttpJsonResponse Error(int statusCode, const std::string& statusDescription, const std::string& message)
	{
		nlohmann::json errorContent;
		errorContent["error"] = message;
		return HttpJsonResponse(statusCode, statusDescription, errorContent);
	}

	std::string statusDescription;
	int statusCode;
	nlohmann::json content;
};

typedef std::function<HttpJsonResponse(const HttpJsonRequest&)> JsonRequestHandler;


void SendJsonResponse(const HttpJsonResponse& jsonResponse, std::shared_ptr<SimpleServer::Response> response)
{
	std::string content;
	if (!jsonResponse.content.empty())
	{
		content = jsonResponse.content.dump(1);
	}
	*response << "HTTP/1.1 " << jsonResponse.statusCode << " " << jsonResponse.statusDescription << "\r\n";
	*response << "Access-Control-Allow-Origin: " << ALLOWED_CORS_ORIGIN << "\r\n";
	if (!content.empty())
	{
		*response << "Content-Length: " << content.length() << "\r\n";
		*response << "Content-Type: application/json; charset=utf-8\r\n";
	}
	*response << "\r\n";
	*response << content;
}

network::uri BuildRequestUri(const SimpleServer::Request& request)
{
	// Per HTTP/1.1 the host header is required, and should be used to build the request URL
	auto hostIt = request.header.find("Host");
	if (hostIt == request.header.end())
	{
		throw std::runtime_error("Missing required Host header");
	}
	// TODO: use network::uri::decode() to decode % encoding, note that this can only be done on raw strings coming out of the uri
	return network::uri("http://" + hostIt->second + request.path);
}

/**
 * Returns a request handler from the given JsonRequest -> JsonResponse handler that will correctly handle JSON requests
*/
RequestHandler JsonHandler(JsonRequestHandler handler)
{
	return [=](std::shared_ptr<SimpleServer::Response> response, std::shared_ptr<SimpleServer::Request> request) {
		try
		{
			HttpJsonRequest jsonRequest(*request, BuildRequestUri(*request));
			if (request->method == "POST" || request->method == "PUT")
			{
				// TODO: Check JSON content type
				// See also https://github.com/nlohmann/json/issues/244 as more specific exception types are coming
				try
				{
					jsonRequest.content = nlohmann::json::parse(request->content);
				}
				catch (std::exception& e)
				{
					const auto error = HttpJsonResponse::Error(400, "Bad Request", e.what());
					SendJsonResponse(error, response);
					return;
				}
			}
			const auto& jsonResponse = handler(jsonRequest);
			SendJsonResponse(jsonResponse, response);
		}
		catch (std::exception& e)
		{
			const auto error = HttpJsonResponse::Error(500, "Internal Server Error", e.what());
			SendJsonResponse(error, response);
		}
		catch (...)
		{
			const auto error = HttpJsonResponse::Error(500, "Internal Server Error", "Unknown Error");
			SendJsonResponse(error, response);
		}
	};
}

network::uri MakeUrlWithAuthority(const network::uri& original, std::string newPath)
{
	network::uri_builder builder;
	builder.scheme(original.scheme().to_string());
	builder.authority(original.authority().to_string());
	builder.path(newPath);
	return builder.uri();
}

nlohmann::json ToJson(const bslib::file::FileBackupRunEvent& runEvent)
{
	nlohmann::json result;
	result["action"] = bslib::file::ToString(runEvent.action);
	result["date_utc"] = bslib::ToIso8601Utc(runEvent.dateTimeUtc);
	return result;
}

nlohmann::json ToJson(const bslib::file::FileEvent& fileEvent)
{
	nlohmann::json result;
	result["action"] = bslib::file::ToString(fileEvent.action);
	result["type"] = bslib::file::ToString(fileEvent.type);
	result["path"] = fileEvent.fullPath.ToString();
	result["date_utc"] = bslib::ToIso8601Utc(fileEvent.dateTimeUtc);
	return result;
}

nlohmann::json ToJson(const bslib::file::FileBackupRunReader::BackupSummary& backup, bool includeEvents)
{
	nlohmann::json result;
	result["id"] = backup.runId.ToString();
	result["modified_files_count"] = backup.modifiedFilesCount;
	result["total_size_bytes"] = backup.totalSizeBytes;
	result["started_on_utc"] = bslib::ToIso8601Utc(backup.startedUtc);
	if (backup.finishedUtc)
	{
		result["finished_on_utc"] = bslib::ToIso8601Utc(backup.finishedUtc.value());
	}
	else
	{
		result["finished_on_utc"] = nullptr;
	}
	if (includeEvents)
	{
		auto backupEvents = nlohmann::json::array();
		for (const auto& ev : backup.backupRunEvents)
		{
			backupEvents.push_back(ToJson(ev));
		}
		result["backup_events"] = backupEvents;
	}
	return result;
}

nlohmann::json ToJson(const bslib::file::VirtualFile& file)
{
	nlohmann::json result;
	result["pathId"] = file.pathId;
	const auto filename = file.fullPath.GetFilename();
	if (!filename.empty())
	{
		result["name"] = filename;
	}
	else
	{
		result["name"] = file.fullPath.ToNormalString();
	}
	result["type"] = bslib::file::ToString(file.type);
	if (file.matchedFileEvent)
	{
		result["matched_file_event"] = ToJson(file.matchedFileEvent.value());
	}
	return result;
}

struct PagingParameters
{
	PagingParameters(unsigned skip, unsigned pageSize)
		: skip(skip)
		, pageSize(pageSize)
	{
	}

	const unsigned skip;
	const unsigned pageSize;
};

PagingParameters GetPagingParameters(const HttpJsonRequest& request)
{
	unsigned skip = 0;
	unsigned pageSize = 30;
	const auto queryParameters = request.GetQueryParameters();
	{
		auto it = queryParameters.find("skip");
		if (it != queryParameters.end())
		{
			skip = boost::lexical_cast<unsigned>(it->second);
		}
	}
	{
		auto it = queryParameters.find("pageSize");
		if (it != queryParameters.end())
		{
			pageSize = boost::lexical_cast<unsigned>(it->second);
		}
	}

	return PagingParameters(skip, pageSize);
}

template<typename T, typename C = std::vector<T>>
C ParseCommaList(const std::string& source)
{
	C result;
	boost::char_separator<char> sep(",");
	boost::tokenizer<boost::char_separator<char>>  tokens(source, sep);
	for (const auto& token : tokens)
	{
		result.emplace(boost::lexical_cast<T>(token));
	}
	return result;
}

}

HttpServer::HttpServer(
	int port,
	bslib::Backup& backup,
	bslib::blob::BlobStoreManager& blobStoreManager,
	JobExecutor& jobExecutor)
	: _backup(backup)
	, _blobStoreManager(blobStoreManager)
	, _jobExecutor(jobExecutor)
	, _simpleServer(port, /* number of threads = */ 1)
{
	_simpleServer.config.address = "127.0.0.1";
	_simpleServer.config.reuse_address = true;

	_simpleServer.default_resource["OPTIONS"] = [&](std::shared_ptr<SimpleServer::Response> response, std::shared_ptr<SimpleServer::Request> request) {
		*response << "HTTP/1.1 200 OK\r\n" <<
			"Access-Control-Allow-Origin: " << ALLOWED_CORS_ORIGIN << "\r\n" <<
			"Access-Control-Allow-Methods: *\r\n" <<
			"Access-Control-Allow-Headers: Content-Type, Accept\r\n" <<
			"Access-Control-Max-Age: 86400\r\n\r\n";
	};

	_simpleServer.default_resource["GET"] = [&](std::shared_ptr<SimpleServer::Response> response, std::shared_ptr<SimpleServer::Request> request) {
		*response << "HTTP/1.1 404 Not Found\r\n\r\n";
	};

	_simpleServer.resource["^/file$"]["POST"] = JsonHandler([&](const HttpJsonRequest& request) {
		const auto inputPath = request.content.find("path");
		if (inputPath == request.content.end() || !inputPath->is_string())
		{
			return HttpJsonResponse::Error(400, "Bad Request", "path is required");
		}
		_jobExecutor.Queue(std::make_unique<FileBackupJob>(inputPath->get<std::string>()));
		return HttpJsonResponse(202, "Accepted");
	});

	// Test API that takes JSON and deserializes it, then sends it back as JSON
	_simpleServer.resource["^/api/ping.*"]["POST"] = JsonHandler([](const HttpJsonRequest& request) {
		nlohmann::json responseContent(request.content);

		// expand the API
		responseContent["_url"]["authority"] = request.uri.authority().to_string();
		responseContent["_url"]["host"] = request.uri.host().to_string();
		responseContent["_url"]["port"] = request.uri.port().to_string();
		responseContent["_url"]["path"] = request.uri.path().to_string();
		responseContent["_url"]["query"] = request.uri.query().to_string();
		responseContent["_url"]["queryParameters"] = nlohmann::json::object();

		for (const auto& qp : request.GetQueryParameters())
		{
			responseContent["_url"]["queryParameters"][qp.first] = qp.second;
		}

		return HttpJsonResponse(200, "OK", responseContent);
	});

	_simpleServer.resource["^/api/stores$"]["GET"] = JsonHandler([&](const HttpJsonRequest& request) {
		const auto stores = _blobStoreManager.GetStores();
		nlohmann::json storesResult;
		for (const auto& store : stores)
		{
			nlohmann::json storeResult;
			storeResult["id"] = store->GetId().ToString();
			storeResult["type"] = store->GetTypeString();
			storeResult["settings"] = store->ConvertToJson();
			storesResult.push_back(storeResult);
		}
		return HttpJsonResponse(200, "OK", storesResult);
	});

	_simpleServer.resource["^/api/stores$"]["POST"] = JsonHandler([&](const HttpJsonRequest& request) {
		const auto inputType = request.content.find("type");
		const auto inputSettings = request.content.find("settings");
		if (inputType == request.content.end() || !inputType->is_string())
		{
			return HttpJsonResponse::Error(400, "Bad Request", std::string("type is required"));
		}
		if (inputSettings == request.content.end() || !inputSettings->is_object())
		{
			return HttpJsonResponse::Error(400, "Bad Request", std::string("settings are required"));
		}
		const auto& store = _blobStoreManager.AddBlobStore(inputType->get<std::string>(), *inputSettings);
		blobStoreManager.SaveToSettingsFile();
		nlohmann::json created;
		created["id"] = store.GetId().ToString();
		created["type"] = store.GetTypeString();
		created["settings"] = store.ConvertToJson();
		return HttpJsonResponse(201, "Created", created);
	});

	_simpleServer.resource[R"(^/api/files/backups(\?.*|$))"]["GET"] = JsonHandler([&](const HttpJsonRequest& request) {
		const auto paging = GetPagingParameters(request);
		auto uow = _backup.CreateUnitOfWork();
		const auto reader = uow->CreateFileBackupRunReader();
		const bslib::file::FileBackupRunSearchCriteria criteria;
		const auto page = reader->Search(criteria, paging.skip, paging.pageSize);
		auto backupsResult = nlohmann::json::array();
		for (const auto& backup : page.backups)
		{
			auto backupResult = ToJson(backup, false);
			backupResult["details_url"] = MakeUrlWithAuthority(request.uri, "/api/files/backups/" + backup.runId.ToString()).string();
			backupsResult.push_back(backupResult);
		}
		nlohmann::json result;
		result["backups"] = backupsResult;
		result["page_size"] = paging.pageSize;
		result["total_backups"] = page.totalBackups;
		network::uri nextPageUrl;
		{
			network::uri_builder builder(request.uri);
			// Work around https://github.com/cpp-netlib/uri/issues/91
			if (request.uri.has_query())
			{
				builder.clear_query();
			}
			builder.append_query_key_value_pair("skip", std::to_string(page.nextPageSkip));
			builder.append_query_key_value_pair("pageSize", std::to_string(paging.pageSize));
			nextPageUrl = builder.uri();
		}
		result["next_page_url"] = nextPageUrl.string();
		return HttpJsonResponse(200, "OK", result);
	});

	_simpleServer.resource["^/api/files/backups/([^/]*)$"]["GET"] = JsonHandler([&](const HttpJsonRequest& request) {
		std::string match = request.originalRequest.path_match[1];
		const bslib::Uuid runId(match);
		auto uow = _backup.CreateUnitOfWork();
		const auto reader = uow->CreateFileBackupRunReader();
		bslib::file::FileBackupRunSearchCriteria criteria;
		criteria.runId = runId;
		const auto page = reader->Search(criteria, 0, 1, true);
		if (page.backups.empty())
		{
			return HttpJsonResponse::Error(404, "Not Found", "Backup with id " + match + " not found");
		}
		auto result = ToJson(page.backups[0], true);
		result["file_events_url"] = MakeUrlWithAuthority(request.uri, "/api/files/backups/" + match + "/fileevents").string();
		return HttpJsonResponse(200, "OK", result);
	});

	_simpleServer.resource["^/api/files/backups/([a-zA-Z0-9-]*)/fileevents.*$"]["GET"] = JsonHandler([&](const HttpJsonRequest& request) {
		const auto paging = GetPagingParameters(request);
		std::string match = request.originalRequest.path_match[1];
		const bslib::Uuid runId(match);
		auto uow = _backup.CreateUnitOfWork();
		const auto finder = uow->CreateFileFinder();
		bslib::file::FileEventSearchCriteria criteria;
		criteria.runId = runId;
		const auto page = finder->SearchEvents(criteria, paging.skip, paging.pageSize);
		auto fileEventsResult = nlohmann::json::array();
		for (const auto& fileEvent : page.events)
		{
			fileEventsResult.push_back(ToJson(fileEvent));
		}
		nlohmann::json result;
		result["file_events"] = fileEventsResult;
		result["page_size"] = paging.pageSize;
		result["total_file_events"] = page.totalEvents;
		network::uri nextPageUrl;
		{
			network::uri_builder builder(request.uri);
			// Work around https://github.com/cpp-netlib/uri/issues/91
			if (request.uri.has_query())
			{
				builder.clear_query();
			}
			builder.append_query_key_value_pair("skip", std::to_string(page.nextPageSkip));
			builder.append_query_key_value_pair("pageSize", std::to_string(paging.pageSize));
			nextPageUrl = builder.uri();
		}
		result["next_page_url"] = nextPageUrl.string();
		return HttpJsonResponse(200, "OK", result);
	});

	// GET /api/files/browse/(:pathId)?at=DATE&skip=N&pageSize=M
	_simpleServer.resource["^/api/files/browse(?:/([0-9]+))?[^/]*$"]["GET"] = JsonHandler([&](const HttpJsonRequest& request) {
		const auto paging = GetPagingParameters(request);
		boost::optional<boost::posix_time::ptime> at;
		const auto queryParameters = request.GetQueryParameters();
		{
			auto it = queryParameters.find("at");
			if (it != queryParameters.end())
			{
				at = bslib::FromIso8601Utc(it->second);
			}
		}
		auto uow = _backup.CreateUnitOfWork();
		const auto browser = uow->CreateVirtualFileBrowser(at);
		const auto pathIdMatch = request.originalRequest.path_match[1];
		std::vector<bslib::file::VirtualFile> files;
		if (!pathIdMatch.matched)
		{
			files = browser->ListRoots(paging.skip, paging.pageSize);
		}
		else
		{
			const auto pathId = boost::lexical_cast<unsigned>(pathIdMatch.str());
			files = browser->ListContents(pathId, paging.skip, paging.pageSize);
		}
		auto filesResult = nlohmann::json::array();
		for (const auto& file : files)
		{
			filesResult.push_back(ToJson(file));
		}
		nlohmann::json result;
		result["files"] = filesResult;
		result["page_size"] = paging.pageSize;
		network::uri nextPageUrl;
		{
			network::uri_builder builder(request.uri);
			// Work around https://github.com/cpp-netlib/uri/issues/91
			if (request.uri.has_query())
			{
				builder.clear_query();
			}
			const auto nextPageSkip = paging.skip + std::min(static_cast<unsigned>(files.size()), paging.pageSize);
			builder.append_query_key_value_pair("skip", std::to_string(nextPageSkip));
			builder.append_query_key_value_pair("pageSize", std::to_string(paging.pageSize));
			nextPageUrl = builder.uri();
		}
		result["next_page_url"] = nextPageUrl.string();
		return HttpJsonResponse(200, "OK", result);
	});

	_simpleServer.resource["^/api/files/browse/([0-9,]*)/descendantmatches.*"]["GET"] = JsonHandler([&](const HttpJsonRequest& request) {
		const auto paging = GetPagingParameters(request);
		boost::optional<boost::posix_time::ptime> at;
		const auto queryParameters = request.GetQueryParameters();
		{
			auto it = queryParameters.find("at");
			if (it != queryParameters.end())
			{
				at = bslib::FromIso8601Utc(it->second);
			}
		}
		const auto pathIdsMatch = request.originalRequest.path_match[1];
		const auto pathIds = ParseCommaList<int64_t, std::unordered_set<int64_t>>(pathIdsMatch.str());
		auto uow = _backup.CreateUnitOfWork();
		const auto browser = uow->CreateVirtualFileBrowser(at);
		const auto counts = browser->CountNestedMatches(pathIds);
		nlohmann::json countsResult = nlohmann::json::array();
		for (const auto& kv : counts)
		{
			nlohmann::json pathIdCount;
			pathIdCount["pathId"] = kv.first;
			pathIdCount["count"] = kv.second;
			countsResult.push_back(pathIdCount);
		}
		nlohmann::json result;
		result["counts"] = countsResult;
		return HttpJsonResponse(200, "OK", result);
	});

	// Start the server in a background thread, as it blocks while it accepts connections
	_serverThread = std::thread(&HttpServer::Run, this);
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
