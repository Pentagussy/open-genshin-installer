#pragma once

#include "config.hpp"
#include <optional>

namespace api {

// Performs HTTPS GET request using libcurl
std::string httpGet(const std::string& url);

// Fetches the latest Genshin Impact package manifest from the HoYoPlay CDN API
std::optional<config::GameManifest> fetchLatestManifest();

} // namespace api
