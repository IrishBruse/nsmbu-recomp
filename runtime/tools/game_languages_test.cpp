

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include "game_languages.h"
namespace config { std::string game_dir, save_dir; }
void log_msg(const char*, ...) {}
int main(int argc, char** argv) {
    namespace fs = std::filesystem;
    const fs::path root = argc > 1 ? fs::path(argv[1]) : fs::temp_directory_path() / "nsmbu_game_languages_test";
    fs::remove_all(root);
    const fs::path pack = root / "content" / "Common" / "PACK";
    fs::create_directories(pack);
    for (const char* f : {"permanent_2d_UsEnglish.pack", "permanent_2d_usfrench.pack", "permanent_2d_UsSpanish.pack",
                          "permanent_3d.pack", "permanent_2d_UsGerman.pack.bak"})
        std::ofstream(pack / f) << "x";
    config::game_dir = root.string();
    assert((game_lang::available() == std::vector<int>{1, 2, 5}));
    assert(game_lang::region() == "USA");
    assert(game_lang::is_available(1) && game_lang::is_available(5) && !game_lang::is_available(3) && !game_lang::is_available(0));
    assert(game_lang::usable(2) == 2 && game_lang::usable(3) == 1 && game_lang::usable(10) == 1);
    assert(std::string(game_lang::name(11)) == "Chinese (Taiwan)" && std::string(game_lang::name(12)) == "?");
    assert(game_lang::started() == -1);
    game_lang::set_started(5);
    assert(game_lang::started() == 5 && !game_lang::current().pack);

    assert(game_lang::source_packs().empty() && !game_lang::choose(3, game_lang::kEurope).pack);
    assert(game_lang::choose(3, game_lang::kEurope).language == 1);
    assert(fs::path(game_lang::sources_dir()) == root.parent_path() / "game-lang");
    config::game_dir = (root / "").string();
    assert(fs::path(game_lang::sources_dir()) == root.parent_path() / "game-lang");
#ifdef _WIN32
    _putenv_s("NSMBU_LANG_DIR", "elsewhere");
#else
    setenv("NSMBU_LANG_DIR", "elsewhere", 1);
#endif
    assert(game_lang::sources_dir() == "elsewhere");
    assert(game_lang::region_from_code("EU") == game_lang::kEurope && game_lang::region_from_code("Japan") == game_lang::kJapan);
    assert(game_lang::region_from_code("us") == game_lang::kUsa && game_lang::region_from_code("") == game_lang::kNoRegion);
    assert(std::string(game_lang::region_code(game_lang::kEurope)) == "eu" && std::string(game_lang::region_code(0)).empty());

    for (const char* n : {"Lukas", "Max", "Heinz", "LUKAS", "MAX", "HEINZ"}) assert(std::string(game_lang::german_genitive_suffix(n)) == "'");
    for (const char* n : {"Link", "Anna", "", "Lukaß", "Zelda"}) assert(std::string(game_lang::german_genitive_suffix(n)) == "s");

    for (int l = 0; l < 6; l++) assert(game_lang::options_language(l) == "\0\0\2\1\4\3"[l]);
    fs::remove_all(root);
    puts("game_languages_test: USA packs, region, fallback to English, language source folder passed");
}
