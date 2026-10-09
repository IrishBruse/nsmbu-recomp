

#pragma once
#include <string>
#include <vector>

namespace game_lang {

constexpr int kLanguages = 12;
const char* name(int language);

enum Region : int { kNoRegion = 0, kJapan = 1, kUsa = 2, kEurope = 4 };
const char* region_code(int region);
const char* region_name(int region);
int region_from_code(const std::string& code);

struct Pack {
    int language = -1;
    int region = kNoRegion;
    std::string file;
    std::string host;
    bool source = false;
};

const std::vector<int>& available();
bool is_available(int language);

const std::string& region();

int usable(int language);

std::string sources_dir();

const std::vector<Pack>& source_packs();
const Pack* source_pack(int language, int region);

std::vector<int> source_languages(int region);

struct Start {
    int language = -1;
    int region = kNoRegion;
    const Pack* pack = nullptr;
};

Start choose(int language, int region);

Start current();
void begin(const Start& start);

int started();
void set_started(int language);

std::string redirect(const std::string& guest);

int options_language(int language);

const char* german_genitive_suffix(const std::string& name);

}
