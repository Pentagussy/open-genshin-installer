#include "api.hpp"
#include "downloader.hpp"
#include <iostream>
#include <filesystem>
#include <iomanip>

namespace fs = std::filesystem;

void printUsage() {
    std::cout << "Usage: open-genshin-installer [options]\n\n"
              << "Options:\n"
              << "  --info                Query and display latest official package manifest\n"
              << "  --dir <path>          Installation target directory (default: ./GenshinImpact)\n"
              << "  --lang <language>     Audio language (en-us, ja-jp, zh-cn, ko-kr) (default: en-us)\n"
              << "  --download            Download all required game parts and audio\n"
              << "  --extract             Extract downloaded archives into installation directory\n"
              << "  --run                 Launch GenshinImpact.exe via Wine\n"
              << "  --help                Show this help message\n";
}

int main(int argc, char** argv) {
    std::cout << "====================================================\n";
    std::cout << "  OpenGenshinInstaller (Native Linux FOSS Launcher) \n";
    std::cout << "====================================================\n\n";

    fs::path installDir = fs::current_path() / "GenshinImpact";
    std::string audioLang = "en-us";
    bool doInfo = false;
    bool doDownload = false;
    bool doExtract = false;
    bool doRun = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--info") {
            doInfo = true;
        } else if (arg == "--download") {
            doDownload = true;
        } else if (arg == "--extract") {
            doExtract = true;
        } else if (arg == "--run") {
            doRun = true;
        } else if (arg == "--dir" && i + 1 < argc) {
            installDir = argv[++i];
        } else if (arg == "--lang" && i + 1 < argc) {
            audioLang = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        }
    }

    if (!doInfo && !doDownload && !doExtract && !doRun) {
        doInfo = true;
    }

    auto manifestOpt = api::fetchLatestManifest();
    if (!manifestOpt) {
        std::cerr << "[!] Failed to query official HoYoPlay CDN API.\n";
        return 1;
    }

    const auto& manifest = *manifestOpt;
    std::cout << "\n[+] Latest Official Game Version: " << manifest.version << "\n";
    std::cout << "[+] Multipart Game Archives: " << manifest.gamePackages.size() << " parts\n";

    double totalGb = static_cast<double>(manifest.totalDownloadSize) / (1024.0 * 1024.0 * 1024.0);
    std::cout << "[+] Total Base Game Download Size: " << std::fixed << std::setprecision(2) << totalGb << " GB\n";

    std::cout << "\n[+] Available Voice Audio Packages:\n";
    const config::AudioPackage* selectedAudio = nullptr;
    for (const auto& audio : manifest.audioPackages) {
        double audioGb = static_cast<double>(audio.size) / (1024.0 * 1024.0 * 1024.0);
        bool isSel = (audio.language == audioLang);
        if (isSel) selectedAudio = &audio;
        std::cout << "    - " << audio.language << ": " << audioGb << " GB"
                  << (isSel ? " [SELECTED]" : "") << "\n";
    }

    if (doDownload) {
        fs::path dlDir = installDir / "downloads";
        fs::create_directories(dlDir);

        std::cout << "\n[*] Starting download into: " << dlDir << "\n";

        size_t partIdx = 0;
        for (const auto& pkg : manifest.gamePackages) {
            partIdx++;
            fs::path filename = fs::path(pkg.url).filename();
            fs::path dest = dlDir / filename;

            std::cout << "\n[" << partIdx << "/" << manifest.gamePackages.size() << "] Downloading " << filename << "...\n";
            bool ok = downloader::downloadFile(pkg.url, dest, [](uint64_t dl, uint64_t total) {
                if (total > 0) {
                    double pct = (static_cast<double>(dl) / total) * 100.0;
                    std::cout << "\r    -> " << std::fixed << std::setprecision(1) << pct << "% ("
                              << dl / (1024 * 1024) << " MB / " << total / (1024 * 1024) << " MB)" << std::flush;
                }
            });
            std::cout << "\n";
            if (!ok) {
                std::cerr << "[!] Download failed for: " << filename << "\n";
                return 1;
            }
        }

        if (selectedAudio) {
            fs::path audioName = fs::path(selectedAudio->url).filename();
            fs::path dest = dlDir / audioName;
            std::cout << "\n[*] Downloading audio package " << audioName << "...\n";
            downloader::downloadFile(selectedAudio->url, dest, [](uint64_t dl, uint64_t total) {
                if (total > 0) {
                    double pct = (static_cast<double>(dl) / total) * 100.0;
                    std::cout << "\r    -> " << std::fixed << std::setprecision(1) << pct << "% ("
                              << dl / (1024 * 1024) << " MB / " << total / (1024 * 1024) << " MB)" << std::flush;
                }
            });
            std::cout << "\n";
        }

        std::cout << "\n[+] All packages downloaded successfully!\n";
    }

    if (doExtract) {
        fs::path dlDir = installDir / "downloads";
        std::string part1Name = "GenshinImpact_" + manifest.version + ".zip.001";
        fs::path part1 = dlDir / part1Name;

        std::cout << "\n[*] Extracting multipart game archive...\n";
        if (downloader::extractPackage(part1, installDir)) {
            std::cout << "[+] Game extraction finished!\n";
        } else {
            std::cerr << "[!] Extraction failed. Ensure 7z is installed ('sudo pacman -S p7zip' or 'sudo apt install p7zip-full').\n";
        }
    }

    if (doRun) {
        fs::path gameExe = installDir / "GenshinImpact.exe";
        std::cout << "\n[*] Launching " << gameExe << " with Wine...\n";
        std::string cmd = "wine \"" + gameExe.string() + "\"";
        std::system(cmd.c_str());
    }

    return 0;
}
