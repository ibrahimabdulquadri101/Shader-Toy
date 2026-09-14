#include "FileWatcher.h"

FileWatcher::FileWatcher(const std::string& path) : filePath(path) {
    if (std::filesystem::exists(filePath)) {
        lastModified = std::filesystem::last_write_time(filePath);
    }
}

bool FileWatcher::checkModified() {
    if (!std::filesystem::exists(filePath)) return false;
    
    auto currentModified = std::filesystem::last_write_time(filePath);
    if (currentModified > lastModified) {
        lastModified = currentModified;
        return true;
    }
    return false;
}