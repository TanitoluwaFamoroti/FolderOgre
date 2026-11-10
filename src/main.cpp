#include <iostream>
#include <string>
#include <sstream>

#include <vector>
#include <map>

#include <chrono>

#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <sys/stat.h>
#include <fcntl.h>
#endif

// TODO: Add support for moving files based on their size
// TODO: Add support for moving files based on their name
// TODO: Add JSON configuration file for file type to folder mapping

using namespace std::chrono_literals;
// Constant for the duration of a day in seconds
constexpr time_t DAY_DURATION = std::chrono::duration_cast<std::chrono::seconds>(24h).count();

// Helper function to change a string to lowercase form.
std::string toLower(std::string* s) {
    
    for (size_t i = 0; i < s->size(); i++)
    {
        s->data()[i] = tolower(s->at(i));
    }
    
    return *s;
}

using extensionMapping = std::map<std::string_view, std::string_view>;

// Default mapping of file extensions to their corresponding folder names.
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
    {".doc", "documents"},
    {".docx", "documents"},
    {".pdf", "documents"},
    {".txt", "documents"},
    {".xls", "documents"},
    {".xlsx", "documents"},
    {".ppt", "documents"},
    {".pptx", "documents"},
    {".odt", "documents"},
    {".ods", "documents"},
    {".odp", "documents"},
    {".rtf", "documents"},
    {".csv", "documents"},
    {".html", "documents"},
    {".css", "documents"},
    {".js", "documents"},
    {".json", "documents"},
    {".xml", "documents"},
    {".eml", "emails"},
    {".zip", "archives"},
    {".rar", "archives"},
    {".7z", "archives"},
    {".tar", "archives"},
    {".gz", "archives"},
    {".exe", "executables"},
    {".msi", "executables"},
    {".bat", "executables"},
    {".sh", "executables"}
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

// Enumeration of possible comparisons
enum Comparison {
    INVALID,
    LESSER,
    EQUALS,
    GREATER
};

// Data structure for storing extension sorting rules.
struct ExtensionSortRule {
    extensionMapping extMap;
    bool sortUnknownExt;
};

// Data structure for storing size sorting rules.
struct SizeSortRule {
    size_t size;
    Comparison comparison = Comparison::INVALID;
};

// Data structure for storing date sorting parameters/rules.
struct DateSortRule {
    std::chrono::year_month_day date{};
    Comparison comparison = Comparison::INVALID;
};

// Data structure for storing sorting parameters/rules.
struct SortRule
{
    SortMode sortMode;

    ExtensionSortRule extension;

    SizeSortRule fileSize;

    DateSortRule creationDate;

    DateSortRule lastModifyDate;

    DateSortRule lastAccessDate;

    bool moveHiddenFiles = false;
};

// Sort files based on sort rules
void sortFiles(std::string src, std::string dest, SortRule sr) {

    std::vector<std::filesystem::path> files;
    std::filesystem::path srcdir(src);

    for (const auto& entry : std::filesystem::directory_iterator(srcdir)) {
        if (entry.is_regular_file())
        {
            files.push_back(entry);   
        }
    }

    for (const auto& file : files) {
        struct statx file_stat;
        int result = statx(AT_FDCWD, file.c_str(), AT_STATX_SYNC_AS_STAT, STATX_BTIME | STATX_MTIME | STATX_ATIME | STATX_SIZE, &file_stat);
        if (result != 0) {
            std::cerr << "Failed to read file status for file at: " << file << std::endl;
            continue;
        }

        if (sr.moveHiddenFiles && file.filename().string()[0] == '.') {
            std::filesystem::create_directories(dest + "/.hidden");
            std::filesystem::rename(file, dest + "/.hidden/" + file.filename().string());
            continue;
        }

        if (sr.sortMode == SortMode::SORT_EXTENSION) {
            std::map<std::string_view, std::string_view>& extMap = sr.extension.extMap;
            
            auto extensionIt = extMap.find(toLower(&file.extension().string()));

            if (extensionIt != extMap.end()) {
                std::filesystem::create_directories(dest + "/" + extensionIt->second.data());
                std::filesystem::rename(file, dest + "/" + extensionIt->second.data() + "/" + file.filename().string());
            }
            else if (sr.extension.sortUnknownExt) {
                std::filesystem::create_directories(dest + "/unknown");
                std::filesystem::rename(file, dest + "/unknown/" + file.filename().string());
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

           if (sr.creationDate.comparison == Comparison::EQUALS)
           {
                if (file_stat.stx_btime.tv_sec >= lowerTimeBound && file_stat.stx_btime.tv_sec < upperTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Created on " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Created on " + ss.str() + "/" + file.filename().string());
                }    
           }
           else if (sr.creationDate.comparison == Comparison::LESSER)
           {
                if (file_stat.stx_btime.tv_sec < lowerTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Created before " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Created before " + ss.str() + "/" + file.filename().string());
                }    
           }
           else if (sr.creationDate.comparison == Comparison::GREATER)
           {
                if (file_stat.stx_btime.tv_sec >= upperTimeBound) {
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

           if (sr.lastModifyDate.comparison == Comparison::EQUALS)
           {
                if (file_stat.stx_mtime.tv_sec >= lowerTimeBound && file_stat.stx_mtime.tv_sec < upperTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Last modified on " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Last modified on " + ss.str() + "/" + file.filename().string());
                }    
           }
           else if (sr.lastModifyDate.comparison == Comparison::LESSER)
           {
                if (file_stat.stx_mtime.tv_sec < lowerTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Last modified before " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Last modified before " + ss.str() + "/" + file.filename().string());
                }    
           }
           else if (sr.lastModifyDate.comparison == Comparison::EQUALS)
           {
                if (file_stat.stx_mtime.tv_sec >= upperTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Last modified after " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Last modified after " + ss.str() + "/" + file.filename().string());
                }    
           }  
        }
        else if (sr.sortMode == SortMode::SORT_LAST_ACCESS_DATE) {

            time_t lowerTimeBound = std::chrono::system_clock::to_time_t(std::chrono::sys_days(sr.lastAccessDate.date));
            time_t upperTimeBound = lowerTimeBound + DAY_DURATION;

            /*
            Take an input date and covert it into time_t.
            Define bounds for on that day by adding or removing seconds in the day
            Find where the file lands in these bounds and assign it to the correct folder.
            */

            std::stringstream ss;
            ss << sr.lastAccessDate.date;

           if (sr.lastAccessDate.comparison == Comparison::EQUALS)
           {
                if (file_stat.stx_atime.tv_sec >= lowerTimeBound && file_stat.stx_atime.tv_sec < upperTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Last accessed on " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Last accessed on " + ss.str() + "/" + file.filename().string());
                }    
           }
           else if (sr.lastAccessDate.comparison == Comparison::LESSER)
           {
                if (file_stat.stx_atime.tv_sec < lowerTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Last accessed before " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Last accessed before " + ss.str() + "/" + file.filename().string());
                }    
           }
           else if (sr.lastAccessDate.comparison == Comparison::GREATER)
           {
                if (file_stat.stx_atime.tv_sec >= upperTimeBound) {
                    std::filesystem::create_directories(dest + "/" + "Last accessed after " + ss.str());
                    std::filesystem::rename(file, dest + "/" + "Last accessed after " + ss.str() + "/" + file.filename().string());
                }    
           }
        }
        else if (sr.sortMode == SortMode::SORT_SIZE) {
            size_t sizeKb = file_stat.stx_size / 1024;

            if (sr.fileSize.comparison == Comparison::LESSER)
            {
                if (sizeKb < sr.fileSize.size)
                {
                    std::filesystem::create_directories(dest + "/" + "Size less than " + std::to_string(sr.fileSize.size) + "KB");
                    std::filesystem::rename(file, dest + "/" + "Size less than " + std::to_string(sr.fileSize.size) + "KB/" + file.filename().string());
                }
            }
            else if (sr.fileSize.comparison == Comparison::GREATER)
            {
                if (sizeKb >= sr.fileSize.size)
                {
                    std::filesystem::create_directories(dest + "/" + "Size greater than " + std::to_string(sr.fileSize.size) + "KB");
                    std::filesystem::rename(file, dest + "/" + "Size greater than " + std::to_string(sr.fileSize.size) + "KB/" + file.filename().string());
                }
            }
        }
    }
}


bool moveUnknownFiles = false;
bool moveHiddenFiles = false;

std::string help_flag = "-h";
std::string sortExtension_flag = "-e";
std::string moveUnknownFiles_flag = "-u";
std::string moveHiddenFiles_flag = "-f";
std::string sortSize_flag = "-s";
std::string sortCreationDate_flag = "-c";
std::string sortLastModifyDate_flag = "-m";
std::string sortLastAccessDate_flag = "-a";

const std::string lessThanComparison = "lt";
const std::string equalsToComparison = "eq";
const std::string greaterThanComparison = "gt";

bool createdDestinationFolder = false;

constexpr std::string_view timeFormat = "%d/%m/%Y";

int main(int argc, char* argv[]) {

    // Check if the correct number of arguments is provided
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <source_folder> <destination_folder> [options...]" << std::endl;
        return 1;
    }

    SortRule sr;

    // Set options based on command line arguments
    for (size_t i = 3; i < argc; i++)
    {
        if (std::string(argv[i]) == help_flag) {
            std::cout << "Usage: " << argv[0] << " <source_folder> <destination_folder> -e [options...] [-u]" << std::endl;
            std::cout << " " << argv[0] << " <source_folder> <destination_folder> -s <comparison> <date> [options...]" << std::endl;
            std::cout << " " << argv[0] << " <source_folder> <destination_folder> -c <comparison> <date> [options...]" << std::endl;
            std::cout << " " << argv[0] << " <source_folder> <destination_folder> -m <comparison> <date> [options...]" << std::endl;
            std::cout << " " << argv[0] << " <source_folder> <destination_folder> -a <comparison> <date> [options...]" << std::endl;

            std::cout << "Options:" << std::endl;
            std::cout << "  -h                 Show this help message" << std::endl;
            std::cout << "  -e                 Sort files by extension" << std::endl;
            std::cout << "  -u                 Move unknown files to 'unknown' folder" << std::endl;
            std::cout << "  -f                 Move hidden files to '.hidden' folder" << std::endl;
            std::cout << "  -s <comparison> <size>   Sort files by size (comparison: lt, gt)" << std::endl;
            std::cout << "  -c <comparison> <date>   Sort files by creation date (comparison: lt, eq, gt)" << std::endl;
            std::cout << "  -m <comparison> <date>   Sort files by last modify date (comparison: lt, eq, gt)" << std::endl;
            std::cout << "  -a <comparison> <date>   Sort files by last access date (comparison: lt, eq, gt)" << std::endl;
            std::cout << "Size is in kilobytes" << std::endl;
            std::cout << "Date format: " << timeFormat << std::endl;
            return 0;
        }
        else if (std::string(argv[i]) == sortExtension_flag) {
            sr.sortMode = SortMode::SORT_EXTENSION;
            sr.extension = {extensionToFolder, moveUnknownFiles};
        }
        else if (std::string(argv[i]) == moveUnknownFiles_flag) sr.extension.sortUnknownExt = true;
        else if (std::string(argv[i]) == moveHiddenFiles_flag) sr.moveHiddenFiles = true;
        else if (std::string(argv[i]) == sortSize_flag) {
            sr.sortMode = SortMode::SORT_SIZE;

            std::string comparison = argv[i + 1];
            std::string size = argv[i + 2];

            if (std::string(argv[i + 1]) == greaterThanComparison)
            {
                sr.fileSize.comparison = Comparison::GREATER;
            }
            else if (std::string(argv[i + 1]) == lessThanComparison)
            {
                sr.fileSize.comparison = Comparison::LESSER;
            }
            else
            {
                std::cerr << "Invalid comparison argument after option \"" << sortCreationDate_flag << "\": " << argv[i + 1];
                break;
            }

            if (sr.fileSize.comparison == Comparison::INVALID) {
                break;
            }

            std::stringstream ss(size);
            ss >> sr.fileSize.size;
            
            i += 2;
        }
        else if (std::string(argv[i]) == sortCreationDate_flag) {

            sr.sortMode = SortMode::SORT_CREATION_DATE;

            std::string comparison = argv[i + 1];
            std::string date = argv[i + 2];

            if (std::string(argv[i + 1]) == greaterThanComparison)
            {
                sr.creationDate.comparison = Comparison::GREATER;
            }
            else if (std::string(argv[i + 1]) == equalsToComparison)
            {
                sr.creationDate.comparison = Comparison::EQUALS;
            }
            else if (std::string(argv[i + 1]) == lessThanComparison)
            {
                sr.creationDate.comparison = Comparison::LESSER;
            }
            else
            {
                std::cerr << "Invalid comparison argument after option \"" << sortCreationDate_flag << "\": " << argv[i + 1];
                break;
            }

            if (sr.lastModifyDate.comparison == Comparison::INVALID) {
                break;
            }

            std::stringstream ss(date);
            ss >> std::chrono::parse("%d/%m/%Y", sr.creationDate.date);

            i += 2;
        }
        else if (std::string(argv[i]) == sortLastModifyDate_flag) {
            sr.sortMode = SortMode::SORT_LAST_MODIFY_DATE;

            std::string comparison = argv[i + 1];
            std::string date = argv[i + 2];

            if (std::string(argv[i + 1]) == greaterThanComparison)
            {
                sr.lastModifyDate.comparison = Comparison::GREATER;
            }
            else if (std::string(argv[i + 1]) == equalsToComparison)
            {
                sr.lastModifyDate.comparison = Comparison::EQUALS;
            }
            else if (std::string(argv[i + 1]) == lessThanComparison)
            {
                sr.lastModifyDate.comparison = Comparison::LESSER;
            }
            else
            {
                std::cerr << "Invalid comparison argument after option \"" << sortLastModifyDate_flag << "\": " << argv[i + 1];
                break;
            }

            if (sr.lastModifyDate.comparison == Comparison::INVALID) {
                break;
            }

            std::stringstream ss(date);
            ss >> std::chrono::parse("%d/%m/%Y", sr.lastModifyDate.date);

            i += 2;
        }
        else if (std::string(argv[i]) == sortLastAccessDate_flag) {
            sr.sortMode = SortMode::SORT_LAST_ACCESS_DATE;

            std::string comparison = argv[i + 1];
            std::string date = argv[i + 2];

            if (std::string(argv[i + 1]) == greaterThanComparison)
            {
                sr.lastAccessDate.comparison = Comparison::GREATER;
            }
            else if (std::string(argv[i + 1]) == equalsToComparison)
            {
                sr.lastAccessDate.comparison = Comparison::EQUALS;
            }
            else if (std::string(argv[i + 1]) == lessThanComparison)
            {
                sr.lastAccessDate.comparison = Comparison::LESSER;
            }
            else
            {
                std::cerr << "Invalid comparison argument after option \"" << sortLastAccessDate_flag << "\": " << argv[i + 1];
                break;
            }

            if (sr.lastAccessDate.comparison == Comparison::INVALID) {
                break;
            }

            std::stringstream ss(date);
            ss >> std::chrono::parse("%d/%m/%Y", sr.lastAccessDate.date);

            i += 2;
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