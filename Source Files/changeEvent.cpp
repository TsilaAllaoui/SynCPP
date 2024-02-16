#include <changeEvent.h>

std::mutex ChangeEvent::mutex;

ChangeEvent::ChangeEvent(const ChangeEvent::eventType& type, const fs::path& fileSubPath, const std::wstring& folder) : type_(type), fileSubPath_(fileSubPath), folder_(folder)
{
}

ChangeEvent::eventType ChangeEvent::getType()
{
	return type_;
}

fs::path ChangeEvent::getFileSubPath()
{
	return fileSubPath_;
}

fs::path ChangeEvent::getFolder()
{
	return folder_;
}

