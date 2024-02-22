#pragma once

#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <vector>
#include <set>

namespace fs = std::filesystem;

void syncFiles(std::vector<std::wstring>& folders);

void monitorChanges(const std::vector<std::wstring>& folders);
