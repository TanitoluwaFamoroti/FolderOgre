cmake_minimum_required(VERSION 4.2.3)

set(CMAKE_CXX_STANDARD 20)

project(FolderOgre)

add_executable(folderogre)

target_sources(folderogre
    PUBLIC
        src/main.cpp
)