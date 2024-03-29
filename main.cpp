#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <set>

namespace fs = std::filesystem;

std::vector<fs::directory_entry> getFiles(const std::string& folder) {
	std::vector<fs::directory_entry> entries;
	for (auto& entry : fs::recursive_directory_iterator(folder)) {
		entries.emplace_back(entry);
	}
	return entries;
}

void syncFiles(const std::vector<std::string>& folders, std::unordered_map <fs::path, fs::file_time_type>& fileToWTime) {

	std::unordered_map<std::string, std::set<fs::directory_entry>> allEntries;
	std::unordered_map<std::string, std::vector<fs::directory_entry>> folderToFiles;
	for (auto& folder : folders) {
		auto entries = getFiles(folder);
		folderToFiles.emplace(std::make_pair(folder, entries));
		for (auto& entry : entries) {
			allEntries[folder].emplace(entry);
		}
	}

	for (auto& [folder, entries] : folderToFiles) {
		for (auto& [_folder, folderEntries] : allEntries) {
			if (folder != _folder) {
				for (auto& currEntry : folderEntries) {
					try {
						std::string entryPath;
						try {
							entryPath = currEntry.path().string();
						}
						catch (...) {
							std::wstring entryPath = currEntry.path().wstring();
							auto pos = entryPath.find(std::wstring(_folder.begin(), _folder.end()));
							entryPath = std::wstring(folder.begin(), folder.end()) + entryPath.substr(pos + _folder.size());

							fileToWTime.emplace(std::make_pair(entryPath, fs::last_write_time(fs::path(entryPath))));

							if (currEntry.is_regular_file()) {
								fs::copy_file(currEntry, entryPath);
							}
							else {
								fs::create_directory(entryPath);
							}
							continue;
						}
						auto pos = entryPath.find(_folder);
						entryPath = folder + entryPath.substr(pos + _folder.size());

						if (std::find(entries.begin(), entries.end(), fs::path(entryPath)) == entries.end()
							|| fs::last_write_time(currEntry) < fs::last_write_time(entryPath)
							) {

							fileToWTime.emplace(std::make_pair(entryPath, fs::file_time_type::clock::now()));
							std::cout << "Synching " << entryPath << std::endl;
							if (currEntry.is_regular_file()) {
								fs::copy_file(currEntry, entryPath);
							}
							else {
								fs::create_directory(entryPath);
							}
						}
					}
					catch (std::exception& e) {
						continue;
					}
				}
			}
		}
	}

	std::cout << "Files synched successfully acrross the following folders: " << std::endl;
	for (auto& folder : folders) {
		std::cout << folder << ((folder == folders[folders.size() - 1]) ? "." : ", ");
	}
	std::cout << std::endl;
}

int main(int argc, char** argv) {

	if (argc != 3) {
		if (argv[1] != "-h" && argv[1] != "--help") {
			std::cout << "Arguments error. Need at least two folder path." << std::endl;
		}
		std::cout << "Usage: " << std::endl << "\tsyncpp [folder_path1] [folder_path2] ...";
		return 1;
	}

	int timeout = 10;
	if (argc == 4) {
		timeout = std::stoi(argv[3]);
	}

	std::vector<std::string> folders;
	for (int i = 1; i < argc; i++) {
		folders.emplace_back(argv[i]);
	}

	std::vector<std::string> folders = { "D:\\tests\\a", "D:\\tests\\b" };

	std::unordered_map <fs::path, fs::file_time_type> fileToWTime;

	while (true) {
		syncFiles(folders, fileToWTime);
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}

	return 0;
}