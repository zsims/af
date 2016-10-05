#pragma once

#include "bslib/Forest.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

#include <memory>

namespace af {
namespace bslib {
namespace test {
namespace utility {

/**
 * Wrapper around the forest which supports getting the connection string among other useful test things
 */
class TestBackup
{
public:
	TestBackup(const boost::filesystem::path& baseDir);
	void Open();
	void Create();
	void OpenOrCreate();
	void OpenWithNullStore();
	void CreateWithNullStore();
	std::unique_ptr<sqlitepp::ScopedSqlite3Object> ConnectToDatabase() const;
	const boost::filesystem::path& GetDirectoryStorePath() const { return _baseDir; }
	const boost::filesystem::path& GetForestDbPath() const { return _forestPath; }
	bslib::Forest& GetForest();
	void Close();
private:
	boost::filesystem::path _baseDir;
	boost::filesystem::path _forestPath;
	std::unique_ptr<bslib::Forest> _forest;
};

}
}
}
}

