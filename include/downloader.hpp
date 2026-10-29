#pragma once

#include "config.hpp"
#include <filesystem>
#include <functional>

namespace downloader {

using ProgressCallback = std::function<void(uint64_t downloaded, uint64_t total)>;

// Downloads a file from an official HoYoverse CDN URL to local path
bool downloadFile(const std::string& url, const std::filesystem::path& destPath, ProgressCallback progress = nullptr);

// Extracts multipart 7z/zip packages into destination directory
bool extractPackage(const std::filesystem::path& archivePart1, const std::filesystem::path& targetDir);

} // namespace downloader
