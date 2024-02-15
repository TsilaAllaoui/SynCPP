#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <set>

#include <CLI11.hpp>

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

	std::vector<fs::path> erased;
	auto fileToWTimeCopy = fileToWTime;
	for (const auto& [path, lastWriteTime] : fileToWTime) {
		if (!fs::exists(path)) {
			std::cout << "File " << path << " has been deleted." << std::endl;
			erased.emplace_back(path);
			fileToWTimeCopy.erase(path);
		}
	}
	fileToWTime = fileToWTimeCopy;

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
								fs::copy_file(currEntry, entryPath, fs::copy_options::overwrite_existing);
							}
							else {
								fs::create_directory(entryPath);
							}
							continue;
						}
						auto pos = entryPath.find(_folder);
						entryPath = folder + entryPath.substr(pos + _folder.size());

						if (std::find(entries.begin(), entries.end(), fs::path(entryPath)) == entries.end() &&
							std::find(erased.begin(), erased.end(), fs::path(entryPath)) != erased.end()
							) {
							fs::remove(fs::path(entryPath));
							fs::remove(currEntry);
							fileToWTime.erase(entryPath);
							fileToWTime.erase(fs::path(entryPath));
							continue;
						}

						if (std::find(entries.begin(), entries.end(), fs::path(entryPath)) == entries.end()
							|| fs::last_write_time(currEntry) > fs::last_write_time(entryPath)
							) {

							fileToWTime.emplace(std::make_pair(entryPath, fs::file_time_type::clock::now()));

							if (currEntry.is_regular_file()) {
								std::cout << "Synching " << entryPath << std::endl;
								fs::copy_file(currEntry, entryPath, fs::copy_options::overwrite_existing);
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

	CLI::App app;

	int interval = 1;
	app.add_option("-i, --interval", interval, "Set the interval timeout for rechecking files (optional)");

	std::vector<std::string> folders;
	app.add_option("-f, --folders", folders, "Folders to be synched (at least 2 required)")->required()->expected(2);

	bool noDelay = false;
	app.add_flag("-n, --no-delay", noDelay, "Disable wait time interval");

	CLI11_PARSE(app, argc, argv);

	std::unordered_map <fs::path, fs::file_time_type> fileToWTime;

	folders = { "D:\\tests\\a", "D:\\tests\\b" };

	while (true) {
		syncFiles(folders, fileToWTime);
		if (!noDelay) {
			std::this_thread::sleep_for(std::chrono::seconds(interval));
		}
	}

	return 0;
}