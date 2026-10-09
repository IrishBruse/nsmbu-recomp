

#include "../src/quit_prompt.h"

#include <cstdio>
#include <map>
#include <string>

static int g_failed = 0;
#define CHECK(cond)                                                                       \
    do {                                                                                  \
        if (!(cond)) {                                                                    \
            std::fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
            g_failed++;                                                                   \
        }                                                                                 \
    } while (0)

static std::map<std::string, std::string> g_env;
static const char* fake_env(const char* name) {
    auto it = g_env.find(name);
    return it == g_env.end() ? nullptr : it->second.c_str();
}

int main() {
    using namespace quitprompt;

    CHECK(!gameplay_stage(""));
    CHECK(!gameplay_stage("sea_T"));
    CHECK(!gameplay_stage("Name"));
    CHECK(gameplay_stage("sea"));
    CHECK(gameplay_stage("LinkRM"));
    CHECK(gameplay_stage("M_NewD2"));

    g_env.clear();
    CHECK(!suppressed(fake_env));
    for (const char* v : {"NSMBU_HIDDEN_WINDOWS", "NSMBU_EXIT_AT_FRAME"}) {
        g_env = {{v, "1"}};
        CHECK(suppressed(fake_env));
        g_env = {{v, "0"}};
        CHECK(!suppressed(fake_env));
    }
    g_env = {{"NSMBU_EXIT_AT_FRAME", "4000"}};
    CHECK(suppressed(fake_env));
    g_env = {{"NSMBU_NO_HOST_INPUT", "1"}};
    CHECK(suppressed(fake_env));
    g_env = {{"NSMBU_CONTROLS_SELFTEST", "1"}};
    CHECK(suppressed(fake_env));
    g_env = {{"NSMBU_QUIT_PROMPT", "0"}};
    CHECK(suppressed(fake_env));
    g_env = {{"NSMBU_QUIT_PROMPT", "1"}};
    CHECK(!suppressed(fake_env));

    g_env.clear();
    CHECK(test_answer(fake_env) == Answer::None);
    g_env = {{"NSMBU_TEST_QUIT_ANSWER", "quit"}};
    CHECK(test_answer(fake_env) == Answer::Quit);
    g_env = {{"NSMBU_TEST_QUIT_ANSWER", "cancel"}};
    CHECK(test_answer(fake_env) == Answer::Cancel);
    g_env = {{"NSMBU_TEST_QUIT_ANSWER", "save"}};
    CHECK(test_answer(fake_env) == Answer::SaveAndQuit);
    g_env = {{"NSMBU_TEST_QUIT_ANSWER", "maybe"}};
    CHECK(test_answer(fake_env) == Answer::None);

    Request playing;
    playing.game_window = true;
    playing.gameplay = true;
    CHECK(decide(playing) == Action::Ask);

    Request r = playing;
    r.gameplay = false;
    CHECK(decide(r) == Action::Quit);
    r = playing;
    r.game_window = false;
    CHECK(decide(r) == Action::Quit);
    r = playing;
    r.suppressed = true;
    CHECK(decide(r) == Action::Quit);
    r.test_answer = true;
    CHECK(decide(r) == Action::Ask);
    r = playing;
    r.busy = true;
    CHECK(decide(r) == Action::Ignore);
    r.confirmed = true;
    CHECK(decide(r) == Action::Quit);
    r = playing;
    r.confirmed = true;
    CHECK(decide(r) == Action::Quit);

    if (g_failed) {
        std::fprintf(stderr, "quit_prompt_test: %d check(s) failed\n", g_failed);
        return 1;
    }
    std::printf("quit_prompt_test: all checks passed\n");
    return 0;
}
