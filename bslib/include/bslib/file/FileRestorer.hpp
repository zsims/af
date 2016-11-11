#pragma once

#include "bslib/blob/Address.hpp"
#include "bslib/EventManager.hpp"
#include "bslib/file/FileEvent.hpp"
#include "bslib/file/FileRestoreEvent.hpp"
#include "bslib/file/fs/path.hpp"

#include <functional>
#include <vector>
#include <map>
#include <memory>

namespace af {
namespace bslib {
namespace blob {
class BlobInfoRepository;
class BlobStore;
}
namespace file {

/**
 * Restores files and directories from the backup
 */
class FileRestorer
{
public:
	FileRestorer(
		std::shared_ptr<blob::BlobStore> blobStore,
		blob::BlobInfoRepository& blobInfoRepository);

	/**
	 * Restores the given event to the target path
	 * \param fileEvent The event to restore from
	 * \param targetPath The path to restore to
	 */
	void Restore(const FileEvent& fileEvent, const UTF8String& targetPath);

	/**
	 * Restores the given events to the target path
	 * \param fileEvents The events to restore from
	 * \param targetPath The path to restore to
	 */
	void Restore(const std::vector<FileEvent>& fileEvents, const UTF8String& targetPath);
	void Restore(const std::map<fs::NativePath, FileEvent>& fileEvents, const UTF8String& targetPath);

	const std::vector<FileRestoreEvent>& GetEmittedEvents() { return _emittedEvents; }
	EventManager<FileRestoreEvent>& GetEventManager() { return _eventManager; }
private:
	void RestoreFileEvent(const FileEvent& fileEvent, const fs::NativePath& targetPath);
	bool RestoreBlobToFile(const blob::Address& blobAddress, const fs::NativePath& targetPath) const;
	void EmitEvent(const FileRestoreEvent& fileRestoreEvent);

	static bool IsFileEventActionSupported(FileEventAction action);

	const std::shared_ptr<blob::BlobStore> _blobStore;
	const blob::BlobInfoRepository& _blobInfoRepository;
	std::vector<FileRestoreEvent> _emittedEvents;
	EventManager<FileRestoreEvent> _eventManager;
};

}
}
}

