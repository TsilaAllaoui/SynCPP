#include <folderObserver.h>

std::vector<ChangeEvent> FolderObserver::changes_;

FolderObserver::FolderObserver(const std::wstring& folder) : folder_(folder) {
    directory_handle_ = CreateFile(folder.c_str(), FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (directory_handle_ == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open directory: " << GetLastError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    folder_ = folder.back() == L'\\' ? folder.substr(0, folder.size() - 2) : folder;
}

FolderObserver::~FolderObserver() {
    CloseHandle(directory_handle_);
}

void FolderObserver::observeChanges() {
    while (true) {
        DWORD bytes_returned;
        char buffer[4096];
        FILE_NOTIFY_INFORMATION* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);

        if (!ReadDirectoryChangesW(directory_handle_, buffer, sizeof(buffer), TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytes_returned, nullptr, nullptr)) {
            std::cerr << "Failed to read directory changes: " << GetLastError() << std::endl;
            std::exit(EXIT_FAILURE);
        }

        while (info) {
            try {
                std::wstring fileName(info->FileName, info->FileNameLength / sizeof(WCHAR));
                switch (info->Action) {
                case FILE_ACTION_ADDED: {
                    std::lock_guard<std::mutex> lock(ChangeEvent::mutex);
                    auto event = ChangeEvent(ChangeEvent::eventType::ADD, fs::path(fileName), folder_);
                    changes_.emplace_back(event);
                    break;
                }
                case FILE_ACTION_REMOVED: {
                    std::lock_guard<std::mutex> lock(ChangeEvent::mutex);
                    auto event = ChangeEvent(ChangeEvent::eventType::REMOVE, fs::path(fileName), folder_);
                    changes_.emplace_back(event);
                    break;
                }
                case FILE_ACTION_MODIFIED: {
                    if (!fs::is_directory(fs::path(folder_) / fileName)) {
                        std::lock_guard<std::mutex> lock(ChangeEvent::mutex);
                        auto event = ChangeEvent(ChangeEvent::eventType::UPDATE, fs::path(fileName), folder_);
                        changes_.emplace_back(event);
                    }
                    break;
                }
                case FILE_ACTION_RENAMED_NEW_NAME:
                case FILE_ACTION_RENAMED_OLD_NAME: {
                    auto newFileInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<char*>(info) + info->NextEntryOffset);
                    std::wstring newFileName(newFileInfo->FileName, newFileInfo->FileNameLength / sizeof(WCHAR));
                    std::lock_guard<std::mutex> lock(ChangeEvent::mutex);
                    auto event = ChangeEvent(ChangeEvent::eventType::RENAME, fs::path(newFileName), fs::path(fileName), folder_);
                    changes_.emplace_back(event);
                    info = newFileInfo;
                    break;
                }
                default:
                    break;
                }
                if (info->NextEntryOffset == 0) {
                    break;
                }

                info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<char*>(info) + info->NextEntryOffset);
            }
            catch (...) {
                std::cout << "Name error:" << std::endl;
            }
        }
    }
}
