#include "api.hpp"
#include <curl/curl.h>
#include <iostream>
#include <regex>

namespace api {

static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    std::string* str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), total);
    return total;
}

std::string httpGet(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string buffer;
    if (!curl) return buffer;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "HoYoPlay/1.19.0.382 OpenGenshinInstaller");

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "[!] Curl GET failed: " << curl_easy_strerror(res) << "\n";
    }

    curl_easy_cleanup(curl);
    return buffer;
}

std::optional<config::GameManifest> fetchLatestManifest() {
    std::string apiUrl = config::HYP_GLOBAL_API +
        "/hyp/hyp-connect/api/getGamePackages?launcher_id=" + config::GLOBAL_LAUNCHER_ID +
        "&game_ids[]=" + config::GENSHIN_GAME_ID;

    std::cout << "[*] Querying HoYoPlay API: " << apiUrl << "\n";
    std::string json = httpGet(apiUrl);
    if (json.empty()) return std::nullopt;

    config::GameManifest manifest;

    // Extract Version
    std::regex verRegex(R"__("version"\s*:\s*"([^"]+)")__");
    std::smatch verMatch;
    if (std::regex_search(json, verMatch, verRegex) && verMatch.size() > 1) {
        manifest.version = verMatch[1].str();
    } else {
        std::cerr << "[!] Could not extract game version from manifest.\n";
        return std::nullopt;
    }

    // Extract Game Packages
    std::regex pkgRegex(R"__("url"\s*:\s*"(https?://[^"]+GenshinImpact[^"]+\.zip\.\d+)"\s*,\s*"md5"\s*:\s*"([^"]+)"\s*,\s*"size"\s*:\s*"(\d+)")__");
    auto pkgBegin = std::sregex_iterator(json.begin(), json.end(), pkgRegex);
    auto pkgEnd = std::sregex_iterator();

    for (auto it = pkgBegin; it != pkgEnd; ++it) {
        std::smatch match = *it;
        config::PackagePart part;
        part.url = match[1].str();
        part.md5 = match[2].str();
        part.size = std::stoull(match[3].str());
        manifest.totalDownloadSize += part.size;
        manifest.gamePackages.push_back(part);
    }

    // Extract Audio Packages
    std::regex audioRegex(R"__("language"\s*:\s*"([^"]+)"\s*,\s*"url"\s*:\s*"(https?://[^"]+Audio_[^"]+\.zip)"\s*,\s*"md5"\s*:\s*"([^"]+)"\s*,\s*"size"\s*:\s*"(\d+)")__");
    auto audioBegin = std::sregex_iterator(json.begin(), json.end(), audioRegex);
    auto audioEnd = std::sregex_iterator();

    for (auto it = audioBegin; it != audioEnd; ++it) {
        std::smatch match = *it;
        config::AudioPackage audio;
        audio.language = match[1].str();
        audio.url = match[2].str();
        audio.md5 = match[3].str();
        audio.size = std::stoull(match[4].str());
        manifest.audioPackages.push_back(audio);
    }

    return manifest;
}

} // namespace api
