#pragma once

#include "bslib/unicode.hpp"

#include <boost/filesystem/path.hpp>

namespace af {
namespace bslib {

/**
 * Determines where the default backup database lives.
 * \remarks The path may not exist
 * \returns The full path to the database path, otherwise an empty path if the default path cannot be determined
 */
boost::filesystem::path GetDefaultBackupDatabasePath();

/**
 * Determines where the default store settings live.
 * \remarks The path may not exist
 * \returns The full path to the store settings, otherwise an empty path if the default path cannot be determined
 */
boost::filesystem::path GetDefaultStoreSettingsPath();

}
}