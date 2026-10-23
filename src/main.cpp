#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <array>

const std::array<std::string, 6> imageExtensions = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp"};
const std::array<std::string, 5> videoExtensions = {".mp4", ".avi", ".mov", ".mkv", ".flv"};
const std::array<std::string, 5> audioExtensions = {".mp3", ".wav", ".flac", ".aac", ".ogg"};
const std::array<std::string, 5> documentExtensions = {".pdf", ".docx", ".txt", ".xlsx", ".pptx"};

std::string getFileExtension(const std::string& filename) {
    std::filesystem::path filePath(filename);
    return filePath.extension().string();
}


int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <source_folder> <destination_folder>" << std::endl;
        return 1;
    }

    std::filesystem::path folderPath;

    std::filesystem::path destinationPath;

    if (!std::filesystem::exists(argv[1])) {
        std::cerr << "Source folder path does not exist: " << argv[1] << std::endl;
        return 1;
    } 

    if (!std::filesystem::is_directory(argv[1])) {
        std::cerr << "Source folder path is not a directory: " << argv[1] << std::endl;
        return 1;
    }

    folderPath = std::filesystem::path(argv[1]);        

    destinationPath = std::filesystem::path(argv[2]);


    std::vector<std::filesystem::path> folderContents;

    if (folderPath.empty()) {
        std::cerr << "Source folder path is empty." << std::endl;
        return 1;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(folderPath)) {
        folderContents.push_back(entry.path());
    }

    std::string fileSortType;
    bool fileSorted = false;

    if (!std::filesystem::exists(destinationPath)) {
        std::filesystem::create_directories(destinationPath);
    }

    std::filesystem::permissions(destinationPath, std::filesystem::perms::owner_all | std::filesystem::perms::group_all | std::filesystem::perms::others_all, std::filesystem::perm_options::add);

    for (const auto& file : folderContents) {
        std::filesystem::file_type type = std::filesystem::status(file).type();

        for (const auto& ext : imageExtensions) {
            if (getFileExtension(file) == ext) {
                std::filesystem::create_directories(destinationPath.string() + "/images");
                std::filesystem::rename(file, destinationPath.string() + "/images/" + std::filesystem::path(file).filename().string());
                fileSorted = true;
                fileSortType = "image";
                break;
            }
        }
        for (const auto& ext : videoExtensions) {
            if (getFileExtension(file) == ext) {
                std::filesystem::create_directories(destinationPath.string() + "/videos");
                std::filesystem::rename(file, destinationPath.string() + "/videos/" + std::filesystem::path(file).filename().string());
                fileSorted = true;
                fileSortType = "video";
                break;
            }
        }
        for (const auto& ext : audioExtensions) {
            if (getFileExtension(file) == ext) {
                std::filesystem::create_directories(destinationPath.string() + "/audio");
                std::filesystem::rename(file, destinationPath.string() + "/audio/" + std::filesystem::path(file).filename().string());
                fileSorted = true;
                fileSortType = "audio";
                break;
            }
        }
        for (const auto& ext : documentExtensions) {
            if (getFileExtension(file) == ext) {
                std::filesystem::create_directories(destinationPath.string() + "/documents");
                std::filesystem::rename(file, destinationPath.string() + "/documents/" + std::filesystem::path(file).filename().string());
                fileSorted = true;
                fileSortType = "document";
                break;
            }
        }

        if (!fileSorted) {
            std::cout << "File not sorted: " << file << std::endl;
        }
        else {
            std::cout << "File sorted as " << fileSortType << ": " << file << std::endl;
        }

        fileSorted = false; // Reset for next file
    }

    if (destinationPath.empty()) {
        std::cout << "No files were sorted. Please check the source folder and try again." << std::endl;
        std::filesystem::remove_all(destinationPath); // Clean up the empty destination folder
    }
    else {
        std::cout << "Files have been sorted into: " << destinationPath << std::endl;
    }
}