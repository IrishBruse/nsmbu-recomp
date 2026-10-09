

#pragma once
#include <cstring>
#include <string>

namespace quitprompt {

constexpr int kSaveSlot = 1;

inline bool gameplay_stage(const std::string& stage) { return !stage.empty() && stage != "sea_T" && stage != "Name"; }

using Env = const char* (*)(const char*);
inline bool env_on(const char* v) { return v && *v && strcmp(v, "0") != 0; }

inline bool suppressed(Env env) {
    const char* opt = env("NSMBU_QUIT_PROMPT");
    return env_on(env("NSMBU_HIDDEN_WINDOWS")) || env_on(env("NSMBU_EXIT_AT_FRAME")) || env("NSMBU_NO_HOST_INPUT") ||
           env("NSMBU_CONTROLS_SELFTEST") || (opt && !env_on(opt));
}

enum class Answer { None, Quit, Cancel, SaveAndQuit };
inline Answer test_answer(Env env) {
    const char* v = env("NSMBU_TEST_QUIT_ANSWER");
    if (!v) return Answer::None;
    if (!strcmp(v, "quit")) return Answer::Quit;
    if (!strcmp(v, "cancel")) return Answer::Cancel;
    if (!strcmp(v, "save")) return Answer::SaveAndQuit;
    return Answer::None;
}

enum class Action { Quit, Ask, Ignore };
struct Request {
    bool confirmed = false;
    bool busy = false;
    bool game_window = false;
    bool gameplay = false;
    bool suppressed = false;
    bool test_answer = false;
};
inline Action decide(const Request& r) {
    if (r.confirmed) return Action::Quit;
    if (r.busy) return Action::Ignore;
    if (!r.game_window || !r.gameplay) return Action::Quit;
    if (r.suppressed && !r.test_answer) return Action::Quit;
    return Action::Ask;
}

}
