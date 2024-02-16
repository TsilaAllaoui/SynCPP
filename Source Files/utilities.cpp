#include <folderObserver.h>
#include <utilities.h>

void syncFiles(std::vector<std::wstring>& folders) {
	for (auto i = 0; i < folders.size(); i++) {
		if (folders[i].back() == L'\\') {
			folders[i].pop_back();
		}

		for (auto& entry : fs::recursive_directory_iterator(folders[i])) {
			for (auto j = 0; j < folders.size(); j++) {
				if (j == i) continue;

				if (folders[j].back() == L'\\') {
					folders[j].pop_back();
				}

				auto entryPath = entry.path().wstring();
				auto newPath = folders[j] + L"\\" + entryPath.substr(folders[i].size() + 1);
				if (!fs::exists(newPath)) {
					if (fs::is_directory(entry)) {
						fs::create_directory(newPath);
					}
					else {
						fs::copy_file(entry, newPath);
					}
				}
			}
		}
	}
}

void monitorChanges(const std::vector<std::wstring>& folders)
{
	while (true) {
		ChangeEvent::mutex.lock();
		auto changes = FolderObserver::changes_;
		while (!changes.empty()) {
			auto currentEvent = changes.front();

			auto type = currentEvent.getType();
			auto fileSubPath = currentEvent.getFileSubPath();
			auto folder = currentEvent.getFolder();

			switch (currentEvent.getType())
			{
			case ChangeEvent::eventType::ADD:
				for (auto& currFolder : folders) {
					if (folder != currFolder) {
						if (fs::is_directory(folder / fileSubPath)) {
							fs::create_directory(currFolder / fileSubPath);
						}
						else {
							fs::copy_file(folder / fileSubPath, currFolder / fileSubPath);
						}
					}
				}
				changes.pop();
				break;
			case ChangeEvent::eventType::REMOVE:
				changes.pop();
				break;
			case ChangeEvent::eventType::UPDATE:
				changes.pop();
				break;
			default:
				break;
			}
		}
		ChangeEvent::mutex.unlock();
	}
}
