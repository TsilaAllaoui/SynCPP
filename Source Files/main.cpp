#include <unordered_map>
#include <iostream>
#include <string>
#include <thread>

#include <folderObserver.h>
#include <utilities.h>
#include <CLI11.hpp>



int main(int argc, char** argv) {
    CLI::App app;
    
    int interval = 1;
    app.add_option("-i, --interval", interval, "Set the interval timeout for rechecking files (optional)");
    
    std::vector<std::wstring> folders;
    app.add_option("-f, --folders", folders, "Folders to be synched (at least 2 required)")->required()->expected(2);
    
    bool noDelay = false;
    app.add_flag("-n, --no-delay", noDelay, "Disable wait time interval");
    
    CLI11_PARSE(app, argc, argv);
    
    folders = { L"D:\\tests\\a", L"D:\\tests\\b" };

    ///     1 - Synching all folders
    syncFiles(folders);
    

    ///     2 - Monitor changes throught all folders
    std::vector<std::unique_ptr<FolderObserver>> observers;
    std::vector<std::thread> threads;
    for (auto& folder : folders) {
        auto observer = std::make_unique<FolderObserver>(folder);
        observers.emplace_back(std::move(observer));
        threads.emplace_back(std::thread(&FolderObserver::observeChanges, observers.back().get()));
    }

    threads.emplace_back(std::thread(monitorChanges, folders));
    
    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}