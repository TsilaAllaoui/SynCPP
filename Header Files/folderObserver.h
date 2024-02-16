#pragma once

#include <unordered_map>
#include <filesystem>
#include <Windows.h>
#include <iostream>
#include <thread>
#include <queue>

#include <changeEvent.h>

namespace fs = std::filesystem;

class FolderObserver {
private:
    std::wstring folder_;
    HANDLE directory_handle_;

public:
    FolderObserver(const std::wstring& folder);
    FolderObserver(const FolderObserver& obs) {
        folder_ = obs.folder_;
        directory_handle_ = obs.directory_handle_;
    }
    FolderObserver& operator=(const FolderObserver&) = delete;
    ~FolderObserver();

    static std::queue<ChangeEvent> changes_;

    void observeChanges();
};