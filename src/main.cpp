#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <map>
#include <locale>
#include <cstdlib>
#include <stdio.h>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#endif

#include <sys/stat.h>

// Helper function to change a string to lowercase form.
std::string toLower(std::string& s) {
    
    for (size_t i = 0; i < s.size(); i++)
    {
        s.data()[i] = tolower(s[i]);
    }
    
    return s;
}

// TODO: Add support for moving files based on their creation date
// TODO: Add support for moving files based on their last modified date
// TODO: Add support for moving files based on their last accessed date
// TODO: Add support for moving files based on their size
// TODO: Add support for moving files based on their name
// TODO: Add JSON configuration file for file type to folder mapping

const std::map<std::string_view, std::string_view> extensionToFolder = {
    {".jpg", "images"},
    {".jpeg", "images"},
    {".png", "images"},
    {".gif", "images"},
    {".bmp", "images"},
    {".webp", "images"},
    {".mp4", "videos"},
    {".avi", "videos"},
    {".mov", "videos"},
    {".mkv", "videos"},
    {".flv", "videos"},
    {".mp3", "audio"},
    {".wav", "audio"},
    {".flac", "audio"},
    {".aac", "audio"},
    {".ogg", "audio"},
    {".pdf", "documents"},
    {".docx", "documents"},
    {".txt", "documents"},
    {".xlsx", "documents"},
    {".pptx", "documents"}
};

// Data structure for storing relevant file info for later sorting.
struct FileStatus
{
    std::string_view path;
    std::string ext;
    std::time_t creationTime;
    std::time_t lastModifyTime;
    std::time_t lastAccessTime;
    std::size_t fileSize;
    bool isHidden;
};

// Enumeration of sort modes.
enum SortMode {
    NO_SORT,
    SORT_EXTENSION,
    SORT_CREATION_TIME,
    SORT_LAST_MODIFY_TIME,
    SORT_LAST_ACCESS_TIME,
    SORT_SIZE
};

struct SortRule
{
    SortMode sortMode;

    std::map<std::string_view, std::string_view> extensionCategory;

    std::time_t creationTime;

    std::time_t lastModifyTime;

    std::time_t lastAccessTime;

    std::size_t fileSize;
};


// Get file extension of file at path.
std::string getFileExtension(const std::string_view& path) {
    std::filesystem::path filePath(path);
    std::string ext = filePath.extension().string();
    toLower(ext);
    return ext;
}

// Get creation time of file at path.
std::time_t getFileCreationTime(const std::string_view& path) {
    struct stat file_stat;

    if (stat(path.data(), &file_stat) == 0)
    {
        return file_stat.st_ctim.tv_sec;
    }
    else {
        std::cerr << "Failed to read creation time for file at: " << path;
        return -1;
    }
}

// Get last modify time of file at path.
std::time_t getFileLastModifyTime(const std::string_view& path) {
    struct stat file_stat;

    if (stat(path.data(), &file_stat) == 0)
    {
        return file_stat.st_mtim.tv_sec;
    }
    else {
        std::cerr << "Failed to read last modify time for file at: " << path;
        return -1;
    }
}

// Get last access time of file at path.
std::time_t getFileLastAccessTime(const std::string_view& path) {
    struct stat file_stat;

    if (stat(path.data(), &file_stat) == 0)
    {
        return file_stat.st_atim.tv_sec;
    }
    else {
        std::cerr << "Failed to read last modify time for file at: " << path;
        return -1;
    }
}

// Get last size in bytes of file at path.
std::size_t getFileSize(const std::string_view& path) {
    struct stat file_stat;

    if (stat(path.data(), &file_stat) == 0)
    {
        return file_stat.st_size;
    }
    else {
        std::cerr << "Failed to read last modify time for file at: " << path;
        return 0;
    }
}

// Get whether file at path is hidden.
bool getFileIsHidden(const std::string_view& path) {
    std::filesystem::path p {path};

    return p.filename().string()[0] == '.';
}

// Get all relevant info from files
FileStatus getFileInfo(const std::string_view& path) {
    return {
        path,
        getFileExtension(path),
        getFileCreationTime(path),
        getFileLastModifyTime(path),
        getFileLastAccessTime(path),
        getFileSize(path),
        getFileIsHidden(path)
    };
}


// Sort files based on sort rules
void sortFiles(std::string src, std::string dest, SortRule sr) {

    /* 
        Iterate through source folder
        If a file is seen create a FileStatus object
        If a folder is seen put it into a vector for later parsing
        Sort files by highest priority
        then get a list of all files and 
    */

    std::vector<std::filesystem::path> files;
    std::filesystem::path srcdir(src);

    for (const auto& entry : std::filesystem::directory_iterator(srcdir)) {
        if (entry.is_regular_file())
        {
            files.push_back(entry);   
        }
    }

    for (const auto& file : files) {
        FileStatus f = getFileInfo(file.string());

        if (sr.sortMode == SortMode::SORT_EXTENSION) {
            std::map<std::string_view, std::string_view>& extMap = sr.extensionCategory;
            auto extensionIt = extMap.find(f.ext);

            if (extensionIt != extMap.end()) {
                std::filesystem::create_directories(dest + "/" + extensionIt->second.data());
                std::filesystem::rename(file, dest + "/" + extensionIt->second.data() + "/" + file.filename().string());
            }
        }

        if (sr.sortMode == SortMode::SORT_CREATION_TIME) {

        }

        if (sr.sortMode == SortMode::SORT_LAST_MODIFY_TIME) {

        }

        if (sr.sortMode == SortMode::SORT_LAST_ACCESS_TIME) {

        }

        if (sr.sortMode == SortMode::SORT_SIZE) {
            
        }
        

    //     if (file.filename().string()[0] == '.' && moveHiddenFiles) {
    //         std::filesystem::create_directories(dest.string() + "/.hidden");
    //         std::filesystem::rename(file, dest.string() + "/.hidden/" + file.filename().string());
    //         fileSorted = true;
    //         fileSortType = "hidden";
    //     }
    //     else if (extensionIt != extensionToFolder.end()) {
            
    //     }
    //     else if (moveUnknownFiles) {
    //         std::filesystem::create_directories(destinationPath.string() + "/unknown");
    //         std::filesystem::rename(file, destinationPath.string() + "/unknown/" + file.filename().string());
    //         fileSorted = true;
    //         fileSortType = "unknown";
    //     }

    //     if (!fileSorted) {
    //         std::cout << "File not sorted: " << file << std::endl;
    //     }
    //     else {
    //         std::cout << "File sorted as " << fileSortType << ": " << file << std::endl;
    //     }

    //     fileSorted = false; // Reset for next file
    // }

    // if (sortHiddenFiles) {
    //     std::filesystem::path hiddenFolderPath = destinationPath.string() + "/.hidden";

    //     std::vector<std::filesystem::path> hiddenFolderContents;

    //     for (const auto& entry : std::filesystem::directory_iterator(hiddenFolderPath)) {
    //         if (entry.is_regular_file()) {
    //             hiddenFolderContents.push_back(entry.path());
    //         }
    //     }

    //     for (const auto& file : hiddenFolderContents) {
    //         std::string extension = getFileExtension(file);
    //         toLower(extension);

    //         extensionIt = extensionToFolder.find(extension);
            
    //         if (extensionIt != extensionToFolder.end()) {
    //             std::filesystem::create_directories(destinationPath.string() + "/" + extensionIt->second);
    //             std::filesystem::rename(file, destinationPath.string() + "/" + extensionIt->second + "/" + file.filename().string());
    //             fileSorted = true;
    //             fileSortType = extensionIt->second;
    //         }
    //         else if (moveUnknownFiles) {
    //             std::filesystem::create_directories(destinationPath.string() + "/unknown");
    //             std::filesystem::rename(file, destinationPath.string() + "/unknown/" + file.filename().string());
    //             fileSorted = true;
    //             fileSortType = "unknown";
    //         }

    //         if (!fileSorted) {
    //             std::cout << "File not sorted: " << file << std::endl;
    //         }
    //         else {
    //             std::cout << "File sorted as " << fileSortType << ": " << file << std::endl;
    //         }
    //     }
    }
}

bool moveUnknownFiles = false;
bool moveHiddenFiles = false;
bool sortHiddenFiles = false;
bool sortCreationTime = false;
// bool sortLastModifyTime = false;
// bool sortLastAccessTime = false;

std::string moveUnknownFiles_flag = "-u";
std::string moveHiddenFiles_flag = "-h";
std::string sortHiddenFiles_flag = "-H";
std::string sortCreationTime_flag = "-c";
// std::string sortLastModifyTime_flag = "-c";
// std::string sortLastAccessTime_flag = "-c";

bool createdDestinationFolder = false;

int main(int argc, char* argv[]) {

    // Check if the correct number of arguments is provided
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <source_folder> <destination_folder> [options...]" << std::endl;
        return 1;
    }

    // Set options based on command line arguments
    for (size_t i = 3; i < argc; i++)
    {
        if (std::string(argv[i]) == "-u") moveUnknownFiles = true;
        if (std::string(argv[i]) == "-h") moveHiddenFiles = true;
        if (std::string(argv[i]) == "-H") { moveHiddenFiles = true; sortHiddenFiles = true; }
    }
    
    std::vector<std::filesystem::path> folderContents;

    bool fileSorted = false;
    auto extensionIt = extensionToFolder.end();


    if (!std::filesystem::exists(argv[1])) {
        std::cerr << "Source folder path does not exist: " << argv[1] << std::endl;
        return 1;
    } 

    if (!std::filesystem::is_directory(argv[1])) {
        std::cerr << "Source folder path is not a directory: " << argv[1] << std::endl;
        return 1;
    }

    if (std::filesystem::path(argv[1]).empty()) {
        std::cerr << "Source folder path is empty: " << argv[1] << std::endl;
        return 1;
    }

    std::string sourcePath = argv[1];        

    std::string destinationPath = argv[2];

    SortRule sr;
    sr.sortMode = SortMode::SORT_EXTENSION;
    sr.extensionCategory = extensionToFolder;

    sortFiles(sourcePath, destinationPath, sr);

    if (destinationPath.empty()) {
        std::cout << "No files were sorted. Please check the source folder and try again." << std::endl;
        if (createdDestinationFolder) std::filesystem::remove(destinationPath); // Clean up the empty destination folder
    }
    else {
        std::cout << "Files have been sorted into: " << destinationPath << std::endl;
    }
}