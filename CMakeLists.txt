cmake_minimum_required(VERSION 3.16)
project(open_genshin_installer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

find_package(CURL REQUIRED)

set(SOURCES
    src/api.cpp
    src/downloader.cpp
    src/main.cpp
)

add_executable(open-genshin-installer ${SOURCES})
target_link_libraries(open-genshin-installer PRIVATE CURL::libcurl)
