#pragma once

#include <filesystem>

namespace fs = std::filesystem;

class ChangeEvent
{
public:
    enum class eventType { ADD, REMOVE, UPDATE, RENAME };

private:
    ChangeEvent::eventType type_;
    fs::path fileSubPath_;
    fs::path oldfileSubPath_;
    std::wstring folder_;
public:
    ChangeEvent(const ChangeEvent::eventType& type, const fs::path& fileSubPath, const std::wstring& folder);
    ChangeEvent(const ChangeEvent::eventType& type, const fs::path& fileSubPath, const fs::path& oldfileSubPath, const std::wstring& folder);
    ~ChangeEvent() = default;

    ChangeEvent::eventType getType();
    fs::path getFileSubPath();
    fs::path getOldFileSubPath();
    fs::path getFolder();
    static std::mutex mutex;

    bool operator==(const ChangeEvent& e);
};

