#include "downloader.hpp"
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <cstdlib>

namespace downloader {

struct ProgressData {
    ProgressCallback callback;
    curl_off_t lastDl = 0;
};

static int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    ProgressData* p = static_cast<ProgressData*>(clientp);
    if (p && p->callback && dltotal > 0) {
        p->callback(static_cast<uint64_t>(dlnow), static_cast<uint64_t>(dltotal));
    }
    return 0;
}

static size_t fileWriteCallback(void* ptr, size_t size, size_t nmemb, void* stream) {
    std::ofstream* out = static_cast<std::ofstream*>(stream);
    out->write(static_cast<const char*>(ptr), size * nmemb);
    return size * nmemb;
}

bool downloadFile(const std::string& url, const std::filesystem::path& destPath, ProgressCallback progress) {
    std::ofstream out(destPath, std::ios::binary | std::ios::app);
    if (!out.is_open()) {
        std::cerr << "[!] Could not open destination file: " << destPath << "\n";
        return false;
    }

    // Check existing file size for HTTP resume
    uint64_t existingSize = 0;
    if (std::filesystem::exists(destPath)) {
        existingSize = std::filesystem::file_size(destPath);
    }

    CURL* curl = curl_easy_init();
    if (!curl) return false;

    ProgressData pData{progress};

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fileWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &pData);

    if (existingSize > 0) {
        curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, static_cast<curl_off_t>(existingSize));
    }

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    out.close();

    return (res == CURLE_OK);
}

bool extractPackage(const std::filesystem::path& archivePart1, const std::filesystem::path& targetDir) {
    std::filesystem::create_directories(targetDir);

    // Call system 7z to extract multipart archive
    std::string cmd = "7z x -y \"" + archivePart1.string() + "\" -o\"" + targetDir.string() + "\"";
    std::cout << "[*] Executing: " << cmd << "\n";
    int ret = std::system(cmd.c_str());
    return (ret == 0);
}

} // namespace downloader
