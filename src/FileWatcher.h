#ifndef FILE_WATCHER_H
#define FILE_WATCHER_H

#include <string>
#include <filesystem>

class FileWatcher {
public:
    FileWatcher(const std::string& path);
    bool checkModified();

private:
    std::string filePath;
    std::filesystem::file_time_type lastModified;
};

#endif // FILE_WATCHER_H
