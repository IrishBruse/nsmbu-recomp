#pragma once
#include <filesystem>
#include <map>
#include <string>
namespace mods::content {
using Files=std::map<std::string,std::string>;

bool known_pack(const std::string& filename);

std::string pack_language(const std::string& filename);

void set_game_root(const std::filesystem::path& game);
void import_legacy(const std::filesystem::path& stage,const std::string& source_name);
Files index(const std::filesystem::path& directory);

void activate(Files files);
std::string replacement(const std::string& guest, const std::string& mode="rb");
}
