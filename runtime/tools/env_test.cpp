#include "env.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

static int failures = 0;

static void check(bool ok, const char* what) {
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        failures++;
    }
}

static void put_env(const char* name, const char* value, int overwrite) {
#ifdef _WIN32
    if (!overwrite && std::getenv(name)) return;
    _putenv_s(name, value);
#else
    setenv(name, value, overwrite);
#endif
}

int main() {
    check(std::strcmp(env::NO_HOST_INPUT.name(), "NSMBU_NO_HOST_INPUT") == 0, "no_host_input name");
    check(std::strcmp(env::VK_STATS.name(), "NSMBU_VK_STATS") == 0, "vk_stats name");

    env::unset(env::NO_HOST_INPUT);
    put_env(env::NO_HOST_INPUT.name(), "first", 0);
    check(env::NO_HOST_INPUT() && std::strcmp(env::NO_HOST_INPUT(), "first") == 0, "set initial");
    put_env(env::NO_HOST_INPUT.name(), "second", 0);
    check(std::strcmp(env::NO_HOST_INPUT(), "first") == 0, "set no overwrite");
    put_env(env::NO_HOST_INPUT.name(), "second", 1);
    check(std::strcmp(env::NO_HOST_INPUT(), "second") == 0, "set overwrite");
    env::unset(env::NO_HOST_INPUT);
    check(env::NO_HOST_INPUT() == nullptr, "unset");

    if (failures) return 1;
    std::printf("env_test: all checks passed\n");
    return 0;
}
