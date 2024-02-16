#pragma once

#include <filesystem>

namespace fs = std::filesystem;

class ChangeEvent
{
public:
	enum class eventType {ADD, REMOVE, UPDATE};

private:
	ChangeEvent::eventType type_;
	fs::path fileSubPath_;
	std::wstring folder_;

public:
	ChangeEvent(const ChangeEvent::eventType& type,const fs::path& fileSubPath,const std::wstring& folder);
	~ChangeEvent() = default;

	ChangeEvent::eventType getType();
	fs::path getFileSubPath();
	fs::path getFolder();
	static std::mutex mutex;
};

