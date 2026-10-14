#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
namespace mods::archive {
inline constexpr uint64_t kFileLimit=512ull*1024*1024;
inline constexpr uint64_t kTotalLimit=2ull*1024*1024*1024;
void stage(const std::filesystem::path& source,const std::filesystem::path& destination);
bool relative_path(const std::string& path);
}
