#include "bs_daemon_lib/HttpServer.hpp"

#include "bslib_test_util/TestBase.hpp"

// Avoid size_t <-> int64 warnings on Windows x32
#pragma warning( push )
#pragma warning( disable : 4244 )
#include <client_http.hpp>
#pragma warning( pop )

#include <json.hpp>
#include <network/uri.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <algorithm>
#include <random>

typedef SimpleWeb::Client<SimpleWeb::HTTP> HttpClient;

namespace {
std::random_device randomDevice;
std::mt19937 random(randomDevice());
std::uniform_int_distribution<int> randomUniformDistribution(45000, 46000);

int FindFreePort()
{
	// It's not possible to call bind(address, 0) with the simple http server as it doesn't expose the port it binds to
	return randomUniformDistribution(random);
}

}

namespace af {
namespace bs_daemon {
namespace test {

class HttpServerIntegrationTest : public bslib_test_util::TestBase
{
protected:
	HttpServerIntegrationTest()
		: _testPort(FindFreePort())
		, _testAddress("127.0.0.1:" + std::to_string(_testPort))
		, _backup(_testBackup.OpenOrCreate())
		, _jobExecutor(_backup)
		, _httpServer(_testPort, _backup, _testBackup.GetBlobStoreManager(), _jobExecutor)
	{
	}
	const int _testPort;
	const std::string _testAddress;
	bslib::Backup& _backup;
	JobExecutor _jobExecutor;
	HttpServer _httpServer;
};

TEST_F(HttpServerIntegrationTest, UnregisteredEndpoints_Return404)
{
	// Arrange
	HttpClient client(_testAddress);

	// Act
	auto response = client.request("GET", "/match/123");

	// Assert
	EXPECT_EQ(response->status_code, "404 Not Found");
}

TEST_F(HttpServerIntegrationTest, Ping_Success)
{
	// Arrange
	HttpClient client(_testAddress);
	const auto rawContent = u8R"({"fancy":"Сука Блять"})";
	const auto expectedFancy = u8"Сука Блять";
	const std::string queryString("foo=bar&sayHi&something%20with%20spaces%20=yes%20baby&special%3D=%3D");

	// Act
	auto response = client.request("POST", "/api/ping?" + queryString, rawContent);

	// Assert
	ASSERT_EQ(response->status_code, "200 OK");
	const auto json = nlohmann::json::parse(response->content);
	EXPECT_EQ(expectedFancy, json.at("fancy").get<std::string>());
	// TODO: the HttpClient doesn't sent he port in the Host: header
	// EXPECT_EQ(_testAddress, pt.get<std::string>("_url.authority"));
	// EXPECT_EQ(std::to_string(_testPort), pt.get<std::string>("_url.port"));
	const auto url = json.at("_url");
	ASSERT_TRUE(url.is_object());
	EXPECT_EQ("127.0.0.1", url.at("host").get<std::string>());
	EXPECT_EQ("/api/ping", url.at("path").get<std::string>());
	EXPECT_EQ(queryString, url.at("query").get<std::string>());
	const auto queryParameters = url.at("queryParameters");
	ASSERT_TRUE(queryParameters.is_object());
	EXPECT_EQ("bar", queryParameters.at("foo").get<std::string>());
	EXPECT_EQ("", queryParameters.at("sayHi").get<std::string>());
	EXPECT_EQ("yes baby", queryParameters.at("something with spaces ").get<std::string>());
	EXPECT_EQ("=", queryParameters.at("special=").get<std::string>());
}

TEST_F(HttpServerIntegrationTest, Ping_BadRequestOnInvalidJson)
{
	// Arrange
	HttpClient client(_testAddress);
	const auto rawContent = u8R"({"notvalid})";

	// Act
	auto response = client.request("POST", "/api/ping", rawContent);

	// Assert
	ASSERT_EQ(response->status_code, "400 Bad Request");
	const auto json = nlohmann::json::parse(response->content);
	EXPECT_TRUE(json.at("error").is_string());
}

TEST_F(HttpServerIntegrationTest, PostFilePath_Success)
{
	// Arrange
	HttpClient client(_testAddress);
	
	const auto testPath = GetUniqueTempPath();
	WriteFile(testPath);

	nlohmann::json requestContent;
	requestContent["path"] = testPath.string();
	const auto rawContent = requestContent.dump();

	// Act
	auto response = client.request("POST", "/file", rawContent);

	// Assert
	ASSERT_EQ(response->status_code, "202 Accepted");
}

TEST_F(HttpServerIntegrationTest, PostFilePath_BadRequestIfMissingPath)
{
	// Arrange
	HttpClient client(_testAddress);
	const auto rawContent = u8R"({"notpath":"haha"})";

	// Act
	auto response = client.request("POST", "/file", rawContent);

	// Assert
	ASSERT_EQ(response->status_code, "400 Bad Request");
	const auto json = nlohmann::json::parse(response->content);
	EXPECT_TRUE(json.at("error").is_string());
}

TEST_F(HttpServerIntegrationTest, PostStores_Success)
{
	// Arrange
	HttpClient client(_testAddress);
	const auto testPath = GetUniqueTempPath().string();

	nlohmann::json requestContent;
	requestContent["type"] = "directory";
	requestContent["settings"]["path"] = testPath;
	const auto rawContent = requestContent.dump();

	// Act
	auto response = client.request("POST", "/api/stores", rawContent);

	// Assert
	ASSERT_EQ(response->status_code, "201 Created");
	const auto responseContent = nlohmann::json::parse(response->content);
	EXPECT_EQ("directory", responseContent.at("type").get<std::string>());
	EXPECT_TRUE(responseContent.at("id").is_string());
	EXPECT_EQ(testPath, responseContent.at("settings").at("path").get<std::string>());
}

TEST_F(HttpServerIntegrationTest, PostStores_BadRequestIfMissingType)
{
	// Arrange
	HttpClient client(_testAddress);
	const auto rawContent = u8R"({"nottype":"haha"})";

	// Act
	auto response = client.request("POST", "/api/stores", rawContent);

	// Assert
	ASSERT_EQ(response->status_code, "400 Bad Request");
	const auto responseContent = nlohmann::json::parse(response->content);
	EXPECT_TRUE(responseContent.at("error").is_string());
}

TEST_F(HttpServerIntegrationTest, GetBackups_Success)
{
	// Arrange
	HttpClient client(_testAddress);

	// Record a few backups
	auto uow = _backup.CreateUnitOfWork();
	auto recorder = uow->CreateFileBackupRunRecorder();
	const auto run1 = recorder->Start();
	recorder->Stop(run1);
	const auto run2 = recorder->Start();
	const auto run3 = recorder->Start();
	recorder->Stop(run3);
	const auto run4 = recorder->Start();
	recorder->Stop(run4);
	const auto run5 = recorder->Start();
	recorder->Stop(run5);
	uow->Commit();

	// Act
	// Assert
	network::uri nextUrl;
	{
		auto response = client.request("GET", "/api/files/backups?pageSize=2");
		ASSERT_EQ(response->status_code, "200 OK");
		const auto responseContent = nlohmann::json::parse(response->content);
		EXPECT_EQ(2, responseContent.at("page_size").get<unsigned>());
		EXPECT_EQ(5, responseContent.at("total_backups").get<unsigned>());
		const auto backupsIt = responseContent.find("backups");
		ASSERT_TRUE(backupsIt != responseContent.end()) << "'backups' element is found";
		EXPECT_EQ(2, backupsIt->size());
		{
			const auto backupIt = std::find_if(backupsIt->begin(), backupsIt->end(), [&](const auto& x) { return bslib::Uuid(x.at("id").get<std::string>()) == run5; });
			ASSERT_TRUE(backupIt != backupsIt->end());
			EXPECT_TRUE(backupIt->at("started_on_utc").is_string());
			EXPECT_TRUE(backupIt->at("finished_on_utc").is_string());
			EXPECT_TRUE(backupIt->at("modified_files_count").is_number());
			EXPECT_TRUE(backupIt->at("total_size_bytes").is_number());
		}
		{
			const auto backupIt = std::find_if(backupsIt->begin(), backupsIt->end(), [&](const auto& x) { return bslib::Uuid(x.at("id").get<std::string>()) == run4; });
			ASSERT_TRUE(backupIt != backupsIt->end());
			EXPECT_TRUE(backupIt->at("started_on_utc").is_string());
			EXPECT_TRUE(backupIt->at("finished_on_utc").is_string());
			EXPECT_TRUE(backupIt->at("modified_files_count").is_number());
			EXPECT_TRUE(backupIt->at("total_size_bytes").is_number());
		}
		ASSERT_TRUE(responseContent.at("next_page_url").is_string());
		nextUrl = network::uri(responseContent.at("next_page_url").get<std::string>());
	}

	{
		// The client is pretty crummy, so have to pull apart the URL :/
		const auto nextPath = nextUrl.path().to_string() + "?" + nextUrl.query().to_string();
		auto response = client.request("GET", nextPath);
		ASSERT_EQ(response->status_code, "200 OK");
		const auto responseContent = nlohmann::json::parse(response->content);
		const auto backupsIt = responseContent.find("backups");
		ASSERT_TRUE(backupsIt != responseContent.end()) << "'backups' element is found";
		{
			const auto backupIt = std::find_if(backupsIt->begin(), backupsIt->end(), [&](const auto& x) { return bslib::Uuid(x.at("id").get<std::string>()) == run3; });
			EXPECT_TRUE(backupIt->at("started_on_utc").is_string());
			EXPECT_TRUE(backupIt->at("finished_on_utc").is_string());
		}
		{
			const auto backupIt = std::find_if(backupsIt->begin(), backupsIt->end(), [&](const auto& x) { return bslib::Uuid(x.at("id").get<std::string>()) == run2; });
			EXPECT_TRUE(backupIt->at("started_on_utc").is_string());
			EXPECT_TRUE(backupIt->at("finished_on_utc").is_null());
		}
		ASSERT_TRUE(responseContent.at("next_page_url").is_string());
		nextUrl = network::uri(responseContent.at("next_page_url").get<std::string>());
	}

	{
		const auto nextPath = nextUrl.path().to_string() + "?" + nextUrl.query().to_string();
		auto response = client.request("GET", nextPath);
		ASSERT_EQ(response->status_code, "200 OK");
		const auto responseContent = nlohmann::json::parse(response->content);
		const auto backupsIt = responseContent.find("backups");
		ASSERT_TRUE(backupsIt != responseContent.end()) << "'backups' element is found";
		{
			const auto backupIt = std::find_if(backupsIt->begin(), backupsIt->end(), [&](const auto& x) { return bslib::Uuid(x.at("id").get<std::string>()) == run1; });
			EXPECT_TRUE(backupIt->at("started_on_utc").is_string());
			EXPECT_TRUE(backupIt->at("finished_on_utc").is_string());
		}
	}
}

TEST_F(HttpServerIntegrationTest, GetBackupDetails_Success)
{
	// Arrange
	HttpClient client(_testAddress);

	// Record a few backups
	auto uow = _backup.CreateUnitOfWork();
	auto recorder = uow->CreateFileBackupRunRecorder();
	const auto run1 = recorder->Start();
	recorder->Stop(run1);
	uow->Commit();

	// Act
	auto response = client.request("GET", "/api/files/backups/" + run1.ToString());

	// Assert
	ASSERT_EQ(response->status_code, "200 OK");
	const auto responseContent = nlohmann::json::parse(response->content);
	EXPECT_TRUE(responseContent.at("started_on_utc").is_string());
	EXPECT_TRUE(responseContent.at("finished_on_utc").is_string());
	EXPECT_TRUE(responseContent.at("modified_files_count").is_number());
	EXPECT_TRUE(responseContent.at("total_size_bytes").is_number());
	EXPECT_TRUE(responseContent.at("backup_events").is_array());
	EXPECT_EQ(2, responseContent.at("backup_events").size());
	EXPECT_TRUE(responseContent.at("file_events_url").is_string());
}

TEST_F(HttpServerIntegrationTest, GetBackupDetails_404IfNotFound)
{
	// Arrange
	HttpClient client(_testAddress);

	// Act
	auto response = client.request("GET", "/api/files/backups/00000000-0000-0000-0000-000000000000");

	// Assert
	ASSERT_EQ(response->status_code, "404 Not Found");
}

}
}
}
