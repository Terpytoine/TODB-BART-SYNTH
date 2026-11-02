#pragma once
#include "installer-support.hpp"
#include <cstring>
namespace install {
struct Item { std::string name; std::vector<char> bytes; };
inline std::vector<Item> readPayload(const fs::path& executable) {
    std::ifstream stream(executable, std::ios::binary);
    stream.seekg(0, std::ios::end); auto length = stream.tellg();
    if (length < 80) throw std::runtime_error("Installer payload is missing");
    stream.seekg(length - std::streamoff(16));
    char marker[8]; uint64_t offset = 0; stream.read(marker, 8); stream.read(reinterpret_cast<char*>(&offset), 8);
    if (!stream || std::memcmp(marker, "TODBPACK", 8) || offset < 64 || offset >= uint64_t(length) - 16)
        throw std::runtime_error("Installer payload is invalid");
    stream.seekg(offset); uint32_t count = 0; stream.read(reinterpret_cast<char*>(&count), 4);
    if (!count || count > 10000) throw std::runtime_error("Installer file list is invalid");
    std::set<std::string> names; std::vector<Item> items; uint64_t total = 0;
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t nameLength = 0; uint64_t size = 0;
        stream.read(reinterpret_cast<char*>(&nameLength), 4); stream.read(reinterpret_cast<char*>(&size), 8);
        if (!stream || !nameLength || nameLength > 2048 || size > 250000000 || total + size > 1500000000)
            throw std::runtime_error("Installer file size is invalid");
        Item item; item.name.resize(nameLength); stream.read(item.name.data(), nameLength);
        if (!stream || !validName(item.name) || !names.insert(item.name).second || uint64_t(stream.tellg()) + size > uint64_t(length) - 16)
            throw std::runtime_error("Installer file path is invalid");
        item.bytes.resize(size); stream.read(item.bytes.data(), size);
        if (!stream) throw std::runtime_error("Installer payload is incomplete");
        total += size; items.push_back(std::move(item));
    }
    if (stream.tellg() != length - std::streamoff(16)) throw std::runtime_error("Installer payload has extra data");
    return items;
}
// Stage every byte first, then replace files. Recover previous versions if any replacement fails.
inline void installFiles(const Roots& roots, std::vector<Item> items) {
    std::vector<std::string> names;
    for (auto& item : items) names.push_back(item.name);
    auto manifestPath = roots.app() / manifestName;
    if (fs::exists(manifestPath)) { auto previous = readManifest(manifestPath); names.insert(names.end(), previous.begin(), previous.end()); }
    const std::string manifestEntry = "app/BART SYNTH/.todb-install-files";
    names.push_back(manifestEntry); auto text = manifest(names);
    items.push_back({manifestEntry, std::vector<char>(text.begin(), text.end())});
    for (auto& item : items) { auto destination = roots.file(item.name); if (!unlocked(destination)) throw std::runtime_error("Close BART SYNTH and unload its VST3 before updating"); }
    std::wstring suffix = L".todb-stage-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(GetTickCount64());
    const fs::path appStage = roots.appParent / suffix, pluginStage = roots.pluginParent / suffix;
    if (!safeTarget(appStage, roots.appParent) || !safeTarget(pluginStage, roots.pluginParent)) throw std::runtime_error("Installer staging path is redirected");
    struct Change { fs::path destination, backup; bool hadPrevious; };
    std::vector<Change> changes;
    auto stage = [&](const std::string& name, bool backup) {
        const bool app = name.rfind("app/", 0) == 0;
        return (app ? appStage : pluginStage) / (backup ? L"backup" : L"files") / fs::u8path(name.substr(app ? 4 : 7));
    };
    auto clean = [&] { for (auto& pair : {std::make_pair(appStage, roots.appParent), std::make_pair(pluginStage, roots.pluginParent)}) {
        if (safeTarget(pair.first, pair.second)) { std::error_code error; fs::remove_all(pair.first, error); }
    }};
    try {
        for (auto& item : items) {
            auto staged = stage(item.name, false); fs::create_directories(staged.parent_path());
            std::ofstream out(staged, std::ios::binary); out.write(item.bytes.data(), item.bytes.size()); out.close();
            if (!out) throw std::runtime_error("Windows blocked installer staging");
        }
        for (auto& item : items) {
            auto destination = roots.file(item.name), backup = stage(item.name, true);
            fs::create_directories(destination.parent_path()); const bool previous = fs::exists(destination);
            if (previous) { fs::create_directories(backup.parent_path()); fs::rename(destination, backup); }
            changes.push_back({destination, backup, previous}); fs::rename(stage(item.name, false), destination);
        }
        clean();
    } catch (...) {
        bool recovered = true;
        for (auto it = changes.rbegin(); it != changes.rend(); ++it) {
            std::error_code error; fs::remove(it->destination, error);
            if (it->hadPrevious) { error.clear(); fs::rename(it->backup, it->destination, error); if (error) recovered = false; }
        }
        // Keep a backup if another process prevented recovery, rather than discarding the prior install.
        if (recovered) clean();
        throw;
    }
}
}
