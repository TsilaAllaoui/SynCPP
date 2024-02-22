#include <fstream>

#include <folderObserver.h>
#include <utilities.h>
#include <dtl.hpp>


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
        std::lock_guard<std::mutex> lock(ChangeEvent::mutex);
        auto& changes = FolderObserver::changes_;
        while (!changes.empty()) {
            auto currentEvent = *(changes.begin());

            auto type = currentEvent.getType();
            auto fileSubPath = currentEvent.getFileSubPath();
            auto folder = currentEvent.getFolder();

            try {
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
                    changes.erase(changes.begin());
                    break;
                case ChangeEvent::eventType::REMOVE:
                    for (auto& currFolder : folders) {
                        if (folder != currFolder) {
                            fs::remove(currFolder / fileSubPath);
                        }
                    }
                    changes.erase(changes.begin());
                    break;
                case ChangeEvent::eventType::UPDATE: {

                    std::ifstream sourceFile(folder / fileSubPath);
                    std::stringstream ss;
                    ss << sourceFile.rdbuf();
                    auto source = ss.str();

                    for (auto& currentFolder : folders) {
                        if (currentFolder != folder) {
                            std::ifstream destFile(folder / fileSubPath);
                            std::stringstream ss;
                            ss << destFile.rdbuf();
                            auto dest = ss.str();

                            dtl::Diff<char, std::string> diff(source, dest);
                            diff.onHuge();
                            diff.compose();
                            diff.printSES();
                        }
                    }

                    changes.erase(changes.begin());
                    break;
                }
                case ChangeEvent::eventType::RENAME: {
                    for (auto& currFolder : folders) {
                        if (currFolder != folder) {
                            auto oldFile = currFolder / currentEvent.getOldFileSubPath();
                            auto newFile = currFolder / currentEvent.getFileSubPath();
                            fs::rename(oldFile, newFile);
                        }
                    }
                    changes.erase(changes.begin());
                    break;
                }
                default:
                    changes.erase(changes.begin());
                    break;
                }
            }
            catch (std::exception& e) {
                changes.erase(changes.begin());
            }

        }
    }
}
