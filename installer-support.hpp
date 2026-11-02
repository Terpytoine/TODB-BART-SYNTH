#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <filesystem>
#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <cstdint>
namespace install {
namespace fs = std::filesystem;
constexpr wchar_t registryKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\TODB BART SYNTH";
constexpr wchar_t manifestName[] = L".todb-install-files";
inline bool redirectsPath(const fs::path& path) {
    DWORD attributes = GetFileAttributesW(path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_REPARSE_POINT)) return false;
    HANDLE handle = CreateFileW(path.c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
    if (handle == INVALID_HANDLE_VALUE) return true;
    FILE_ATTRIBUTE_TAG_INFO info{}; bool read = GetFileInformationByHandleEx(handle, FileAttributeTagInfo, &info, sizeof(info));
    CloseHandle(handle);
    // OneDrive cloud placeholders are reparse points, but do not redirect a pathname.
    // Reject junctions/symlinks and other name-surrogate tags, not cloud metadata.
    return !read || (info.ReparseTag & 0x20000000) != 0;
}
inline bool safeTarget(const fs::path& target, const fs::path& root) {
    auto absolute = fs::absolute(target).lexically_normal(), base = fs::absolute(root).lexically_normal();
    auto a = absolute.begin();
    for (auto b = base.begin(); b != base.end(); ++b, ++a)
        if (a == absolute.end() || _wcsicmp(a->c_str(), b->c_str())) return false;
    if (a == absolute.end()) return false;
    // Inspect the entire ancestry, including the chosen root. Never follow redirects.
    for (auto path = absolute; !path.empty(); path = path.parent_path()) {
        if (redirectsPath(path)) return false;
        if (path == path.parent_path()) break;
    }
    return true;
}
inline bool validName(const std::string& name) {
    if (name.empty() || name.size() > 2048 || name.find_first_of("\\:\r\n") != std::string::npos) return false;
    if (name.rfind("app/BART SYNTH/", 0) != 0 && name.rfind("plugin/TODB BART SYNTH.vst3/", 0) != 0) return false;
    auto path = fs::u8path(name);
    if (path.is_absolute() || path.has_root_name()) return false;
    for (auto part : path) if (part == "." || part == ".." || part.empty()) return false;
    for (unsigned char c : name) if (c < 32) return false;
    return path.filename().wstring().back() != L'.' && path.filename().wstring().back() != L' ';
}
inline bool unlocked(const fs::path& file) {
    if (!fs::exists(file)) return true;
    HANDLE handle = CreateFileW(file.c_str(), GENERIC_WRITE | DELETE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) return false;
    CloseHandle(handle); return true;
}
struct Roots {
    fs::path appParent, pluginParent;
    fs::path app() const { return appParent / L"BART SYNTH"; }
    fs::path plugin() const { return pluginParent / L"TODB BART SYNTH.vst3"; }
    fs::path file(const std::string& name) const {
        if (!validName(name)) throw std::runtime_error("Invalid installed file name");
        auto destination = name.rfind("app/", 0) == 0 ? appParent / fs::u8path(name.substr(4)) : pluginParent / fs::u8path(name.substr(7));
        auto root = name.rfind("app/", 0) == 0 ? appParent : pluginParent;
        if (!safeTarget(destination, root)) throw std::runtime_error("Install path is redirected");
        return destination;
    }
};
inline std::vector<std::string> readManifest(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary); std::string line;
    if (!std::getline(stream, line) || line != "TODB_INSTALL_V1") throw std::runtime_error("Installation manifest is unavailable");
    std::vector<std::string> names; std::set<std::string> unique;
    while (std::getline(stream, line)) {
        if (!validName(line) || names.size() >= 10000 || !unique.insert(line).second)
            throw std::runtime_error("Installation manifest is invalid");
        names.push_back(line);
    }
    if (names.empty() || !stream.eof()) throw std::runtime_error("Installation manifest is incomplete");
    return names;
}
inline std::string manifest(const std::vector<std::string>& names) {
    std::string result = "TODB_INSTALL_V1\n";
    std::set<std::string> unique;
    for (auto& name : names) { if (!validName(name)) throw std::runtime_error("Invalid manifest entry"); if (unique.insert(name).second) result += name + "\n"; }
    return result;
}
}
