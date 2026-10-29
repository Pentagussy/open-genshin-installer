#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace config {

// Official HoYoPlay (HYP) Global distribution endpoints
inline const std::string HYP_GLOBAL_API = "https://sg-hyp-api.hoyoverse.com";
inline const std::string GLOBAL_LAUNCHER_ID = "VYTpXlbWo8";
inline const std::string GENSHIN_GAME_ID = "gopR6Cufr3";
inline const std::string GENSHIN_BIZ = "hk4e_global";

struct PackagePart {
    std::string url;
    std::string md5;
    uint64_t size = 0;
    uint64_t decompressedSize = 0;
};

struct AudioPackage {
    std::string language;
    std::string url;
    std::string md5;
    uint64_t size = 0;
};

struct GameManifest {
    std::string version;
    std::vector<PackagePart> gamePackages;
    std::vector<AudioPackage> audioPackages;
    uint64_t totalDownloadSize = 0;
};

} // namespace config
