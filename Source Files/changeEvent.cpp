#include <changeEvent.h>
#include <utilities.h>

std::mutex ChangeEvent::mutex;

ChangeEvent::ChangeEvent(const ChangeEvent::eventType& type, const fs::path& fileSubPath, const std::wstring& folder) : type_(type), fileSubPath_(fileSubPath), folder_(folder)
{
}

ChangeEvent::ChangeEvent(const ChangeEvent::eventType& type, const fs::path& fileSubPath, const fs::path& oldfileSubPath, const std::wstring& folder) : type_(type), fileSubPath_(fileSubPath), oldfileSubPath_(oldfileSubPath), folder_(folder)
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

fs::path ChangeEvent::getOldFileSubPath()
{
    return oldfileSubPath_;
}

fs::path ChangeEvent::getFolder()
{
    return folder_;
}

bool ChangeEvent::operator==(const ChangeEvent& e)
{
    return fileSubPath_ == e.fileSubPath_ && folder_ == e.folder_ && type_ == e.type_;
}
