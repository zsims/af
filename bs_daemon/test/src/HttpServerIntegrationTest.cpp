#include "bs_daemon_lib/HttpServer.hpp"

#include "bslib_test_util/TestBase.hpp"

// Avoid size_t <-> int64 warnings on Windows x32
#pragma warning( push )
#pragma warning( disable : 4244 )
#include <client_http.hpp>
#pragma warning( pop )

#include <json.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

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
		, _httpServer(_testPort, _testBackup.GetBlobStoreManager(), _jobExecutor)
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

}
}
}
