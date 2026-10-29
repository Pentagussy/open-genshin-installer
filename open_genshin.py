#!/usr/bin/env python3
"""
OpenGenshinInstaller (Python CLI)
A clean-room open-source downloader for Genshin Impact on Linux via official HoYoPlay CDN APIs.
"""

import sys
import os
import json
import urllib.request
import argparse
import subprocess

API_URL = "https://sg-hyp-api.hoyoverse.com/hyp/hyp-connect/api/getGamePackages?launcher_id=VYTpXlbWo8&game_ids[]=gopR6Cufr3"

def fetch_manifest():
    headers = {"User-Agent": "HoYoPlay/1.19.0.382 OpenGenshinInstaller"}
    req = urllib.request.Request(API_URL, headers=headers)
    with urllib.request.urlopen(req) as resp:
        data = json.loads(resp.read().decode("utf-8"))
    if data.get("retcode") != 0 or not data.get("data"):
        print(f"[!] API error: {data.get('message')}")
        sys.exit(1)
    return data["data"]["game_packages"][0]["main"]["major"]

def download_file(url, dest_path):
    print(f"\n[*] Downloading: {os.path.basename(dest_path)}")
    cmd = ["curl", "-L", "-C", "-", "-o", dest_path, "--progress-bar", url]
    subprocess.run(cmd, check=True)

def main():
    parser = argparse.ArgumentParser(description="Native Linux Genshin Impact Installer")
    parser.add_argument("--info", action="store_true", help="Display current version and package info")
    parser.add_argument("--download", action="store_true", help="Download official game packages")
    parser.add_argument("--extract", action="store_true", help="Extract packages using 7z")
    parser.add_argument("--lang", default="en-us", choices=["en-us", "ja-jp", "zh-cn", "ko-kr"], help="Audio language pack")
    parser.add_argument("--dir", default="./GenshinImpact", help="Target installation folder")
    args = parser.parse_args()

    manifest = fetch_manifest()
    version = manifest["version"]
    game_pkgs = manifest["game_pkgs"]
    audio_pkgs = manifest["audio_pkgs"]

    total_size = sum(int(p["size"]) for p in game_pkgs) / (1024**3)
    print("=" * 55)
    print("      OpenGenshinInstaller (Native Linux FOSS)       ")
    print("=" * 55)
    print(f"[+] Official Current Version: {version}")
    print(f"[+] Total Base Game Size:    {total_size:.2f} GB ({len(game_pkgs)} multipart archives)")

    print("\n[+] Available Voice Packages:")
    for a in audio_pkgs:
        sz = int(a["size"]) / (1024**3)
        sel = " [SELECTED]" if a["language"] == args.lang else ""
        print(f"    - {a['language']}: {sz:.2f} GB{sel}")

    if args.download:
        dl_dir = os.path.join(args.dir, "downloads")
        os.makedirs(dl_dir, exist_ok=True)

        for idx, pkg in enumerate(game_pkgs, 1):
            fname = os.path.basename(pkg["url"])
            dest = os.path.join(dl_dir, fname)
            print(f"\n[{idx}/{len(game_pkgs)}] {fname}")
            download_file(pkg["url"], dest)

        for a in audio_pkgs:
            if a["language"] == args.lang:
                fname = os.path.basename(a["url"])
                dest = os.path.join(dl_dir, fname)
                print(f"\n[*] Audio: {fname}")
                download_file(a["url"], dest)

        print("\n[+] All downloads complete!")

    if args.extract:
        dl_dir = os.path.join(args.dir, "downloads")
        part1 = os.path.join(dl_dir, f"GenshinImpact_{version}.zip.001")
        if not os.path.exists(part1):
            print(f"[!] Archive part 1 not found: {part1}")
            sys.exit(1)
        print(f"\n[*] Extracting {part1} to {args.dir}...")
        subprocess.run(["7z", "x", "-y", part1, f"-o{args.dir}"], check=True)
        print("[+] Extraction finished successfully!")

if __name__ == "__main__":
    main()
