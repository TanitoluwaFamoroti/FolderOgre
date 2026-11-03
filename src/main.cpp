#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <map>
#include <locale>
#include <cstdlib>
#include <stdio.h>
#include <charconv>
#include <chrono>
#include <ctime>
#ifdef _WIN32
#include <windows.h>
#endif

#include <sys/stat.h>

using namespace std::chrono_literals;
constexpr time_t DAY_DURATION = std::chrono::duration_cast<std::chrono::seconds>(24h).count();

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

using extensionMapping = std::map<std::string_view, std::string_view>;

const extensionMapping extensionToFolder = {
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
    SORT_CREATION_DATE,
    SORT_LAST_MODIFY_DATE,
    SORT_LAST_ACCESS_DATE,
    SORT_SIZE
};

// Enumeration of date sort modes.
enum DateSortMode {
    INVALID,
    BEFORE_DATE,
    ON_DATE,
    AFTER_DATE
};

struct DateSortRule {
    std::chrono::year_month_day date{};
    DateSortMode dateSortMode = DateSortMode::INVALID;
};

// Data structure for storing sorting parameters/rules.
struct SortRule
{
    SortMode sortMode;

    extensionMapping extensionCategory;

    DateSortRule creationDate;

    DateSortRule lastModifyDate;

    DateSortRule lastAccessDate;

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
    FileStatus fs;

    fs.path = path;
    fs.ext = getFileExtension(path);
    fs.creationTime = getFileCreationTime(path);
    fs.lastModifyTime = getFileLastModifyTime(path);
    fs.lastAccessTime = getFileLastAccessTime(path);
    fs.fileSize = getFileSize(path);
    fs.isHidden = getFileIsHidden(path);

    return fs;
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
        else if (sr.sortMode == SortMode::SORT_CREATION_DATE) {

            time_t lowerTimeBound = std::chrono::system_clock::to_time_t(std::chrono::sys_days(sr.creationDate.date));
            time_t upperTimeBound = lowerTimeBound + DAY_DURATION;

            /*
            Take an input date and covert it into time_t.
            Define bounds for on that day by adding or removing seconds in the day
            Find where the file lands in these bounds and assign it to the correct file.
            */

            std::stringstream ss;
            ss << sr.creationDate.date;

           if (sr.creationDate.dateSortMode == DateSortMode::ON_DATE)
           {
                if (f.creationTime >= lowerTimeBound && f.creationTime < upperTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Created on " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Created on " + ss.str() + "/" + file.filename().string());
                }    
           }
           else if (sr.creationDate.dateSortMode == DateSortMode::BEFORE_DATE)
           {
                if (f.creationTime < lowerTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Created before " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Created before " + ss.str() + "/" + file.filename().string());
                }    
           }
           else if (sr.creationDate.dateSortMode == DateSortMode::AFTER_DATE)
           {
                if (f.creationTime >= upperTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Created after " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Created after " + ss.str() + "/" + file.filename().string());
                }    
           }  
        }
        else if (sr.sortMode == SortMode::SORT_LAST_MODIFY_DATE) {

            time_t lowerTimeBound = std::chrono::system_clock::to_time_t(std::chrono::sys_days(sr.lastModifyDate.date));
            time_t upperTimeBound = lowerTimeBound + DAY_DURATION;

            /*
            Take an input date and covert it into time_t.
            Define bounds for on that day by adding or removing seconds in the day
            Find where the file lands in these bounds and assign it to the correct file.
            */

            std::stringstream ss;
            ss << sr.lastModifyDate.date;

           if (sr.lastModifyDate.dateSortMode == DateSortMode::ON_DATE)
           {
                if (f.lastModifyTime >= lowerTimeBound && f.lastModifyTime < upperTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Created on " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Created on " + ss.str() + "/" + file.filename().string());
                }    
           }
           else if (sr.lastModifyDate.dateSortMode == DateSortMode::BEFORE_DATE)
           {
                if (f.lastModifyTime < lowerTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Created before " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Created before " + ss.str() + "/" + file.filename().string());
                }    
           }
           else if (sr.lastModifyDate.dateSortMode == DateSortMode::AFTER_DATE)
           {
                if (f.lastModifyTime >= upperTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Created after " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Created after " + ss.str() + "/" + file.filename().string());
                }    
           }  
        }
        else if (sr.sortMode == SortMode::SORT_LAST_ACCESS_DATE) {

            time_t lowerTimeBound = std::chrono::system_clock::to_time_t(std::chrono::sys_days(sr.lastAccessDate.date));
            time_t upperTimeBound = lowerTimeBound + DAY_DURATION;

            /*
            Take an input date and covert it into time_t.
            Define bounds for on that day by adding or removing seconds in the day
            Find where the file lands in these bounds and assign it to the correct file.
            */

            std::stringstream ss;
            ss << sr.lastAccessDate.date;

           if (sr.lastAccessDate.dateSortMode == DateSortMode::ON_DATE)
           {
                if (f.lastAccessTime >= lowerTimeBound && f.lastAccessTime < upperTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Created on " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Created on " + ss.str() + "/" + file.filename().string());
                }    
           }
           else if (sr.lastAccessDate.dateSortMode == DateSortMode::BEFORE_DATE)
           {
                if (f.lastAccessTime < lowerTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Created before " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Created before " + ss.str() + "/" + file.filename().string());
                }    
           }
           else if (sr.lastAccessDate.dateSortMode == DateSortMode::AFTER_DATE)
           {
                if (f.lastAccessTime >= upperTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Created after " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Created after " + ss.str() + "/" + file.filename().string());
                }    
           }
        }
        else if (sr.sortMode == SortMode::SORT_SIZE) {
            
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

bool sortExtension = false;
bool moveUnknownFiles = false;
bool moveHiddenFiles = false;
bool sortHiddenFiles = false;
bool sortCreationDate = false;
bool sortLastModifyDate = false;
bool sortLastAccessDate = false;

std::string help_flag = "-h";
std::string sortExtension_flag = "-e";
std::string moveUnknownFiles_flag = "-u";
std::string moveHiddenFiles_flag = "-f";
std::string sortHiddenFiles_flag = "-F";
std::string sortCreationDate_flag = "-c";
std::string sortLastModifyDate_flag = "-m";
std::string sortLastAccessDate_flag = "-a";


constexpr std::string_view timeFormat = "%d/%m/%Y";

bool createdDestinationFolder = false;

int main(int argc, char* argv[]) {

    DateSortRule dm;
    SortRule sr;

    // Check if the correct number of arguments is provided
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <source_folder> <destination_folder> [options...]" << std::endl;
        return 1;
    }

    // Set options based on command line arguments
    for (size_t i = 3; i < argc; i++)
    {
        if (std::string(argv[i]) == help_flag) {
            std::cout << "Usage: " << argv[0] << " <source_folder> <destination_folder> [options...]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -h                 Show this help message" << std::endl;
            std::cout << "  -e                 Sort files by extension" << std::endl;
            std::cout << "  -u                 Move unknown files to 'unknown' folder" << std::endl;
            std::cout << "  -f                 Move hidden files to '.hidden' folder" << std::endl;
            std::cout << "  -F                 Sort hidden files by extension" << std::endl;
            std::cout << "  -c <comparison> <date>   Sort files by creation date (comparison: before, on, after)" << std::endl;
            std::cout << "  -m <comparison> <date>   Sort files by last modify date (comparison: before, on, after)" << std::endl;
            std::cout << "  -a <comparison> <date>   Sort files by last access date (comparison: before, on, after)" << std::endl;
            std::cout << "Date format: " << timeFormat << std::endl;
            return 0;
        }
        else if (std::string(argv[i]) == sortExtension_flag) {
            sortExtension = true;
            sr.extensionCategory = extensionToFolder;
        }
        else if (std::string(argv[i]) == moveUnknownFiles_flag) moveUnknownFiles = true;
        else if (std::string(argv[i]) == moveHiddenFiles_flag) moveHiddenFiles = true;
        else if (std::string(argv[i]) == sortHiddenFiles_flag) { moveHiddenFiles = true; sortHiddenFiles = true; }
        else if (std::string(argv[i]) == sortCreationDate_flag) {

            sr.sortMode = SortMode::SORT_CREATION_DATE;

            std::string comparison = argv[i + 1];
            std::string date = argv[i + 2];

            if (std::string(argv[i + 1]) == "after")
            {
                dm.dateSortMode = DateSortMode::AFTER_DATE;
            }
            else if (std::string(argv[i + 1]) == "on")
            {
                dm.dateSortMode = DateSortMode::ON_DATE;
            }
            else if (std::string(argv[i + 1]) == "before")
            {
                dm.dateSortMode = DateSortMode::BEFORE_DATE;
            }
            else
            {
                std::cerr << "Invalid comparison argument after option \"" << sortCreationDate_flag << "\": " << argv[i + 1];
                break;
            }

            if (dm.dateSortMode == DateSortMode::INVALID) {
                break;
            }

            std::stringstream ss(date);
            ss >> std::chrono::parse("%d/%m/%Y", dm.date);

            i += 2;
            sr.creationDate = dm;
        }
        else if (std::string(argv[i]) == sortLastModifyDate_flag) {
            sr.sortMode = SortMode::SORT_LAST_MODIFY_DATE;

            std::string comparison = argv[i + 1];
            std::string date = argv[i + 2];

            if (std::string(argv[i + 1]) == "after")
            {
                dm.dateSortMode = DateSortMode::AFTER_DATE;
            }
            else if (std::string(argv[i + 1]) == "on")
            {
                dm.dateSortMode = DateSortMode::ON_DATE;
            }
            else if (std::string(argv[i + 1]) == "before")
            {
                dm.dateSortMode = DateSortMode::BEFORE_DATE;
            }
            else
            {
                std::cerr << "Invalid comparison argument after option \"" << sortLastModifyDate_flag << "\": " << argv[i + 1];
                break;
            }

            if (dm.dateSortMode == DateSortMode::INVALID) {
                break;
            }

            std::stringstream ss(date);
            ss >> std::chrono::parse("%d/%m/%Y", dm.date);

            i += 2;
            sr.lastModifyDate = dm;
        }
        else if (std::string(argv[i]) == sortLastAccessDate_flag) {
            sr.sortMode = SortMode::SORT_LAST_ACCESS_DATE;

            std::string comparison = argv[i + 1];
            std::string date = argv[i + 2];

            if (std::string(argv[i + 1]) == "after")
            {
                dm.dateSortMode = DateSortMode::AFTER_DATE;
            }
            else if (std::string(argv[i + 1]) == "on")
            {
                dm.dateSortMode = DateSortMode::ON_DATE;
            }
            else if (std::string(argv[i + 1]) == "before")
            {
                dm.dateSortMode = DateSortMode::BEFORE_DATE;
            }
            else
            {
                std::cerr << "Invalid comparison argument after option \"" << sortLastAccessDate_flag << "\": " << argv[i + 1];
                break;
            }

            if (dm.dateSortMode == DateSortMode::INVALID) {
                break;
            }

            std::stringstream ss(date);
            ss >> std::chrono::parse("%d/%m/%Y", dm.date);

            i += 2;
            sr.lastAccessDate = dm;
        }
    }


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

    sortFiles(sourcePath, destinationPath, sr);

    if (!std::filesystem::exists(destinationPath) || std::filesystem::is_empty(destinationPath)) {
        std::cout << "No files were sorted. Please check the source folder and try again." << std::endl;
        if (createdDestinationFolder) std::filesystem::remove(destinationPath); // Clean up the empty destination folder
    }
    else {
        std::cout << "Files have been sorted into: " << destinationPath << std::endl;
    }
}