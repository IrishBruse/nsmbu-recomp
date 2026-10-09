

#include <cstring>
#include <string>

#include "game_languages.h"
#include "guest_addr.h"
#include "runtime.h"

extern "C" {

__attribute__((weak)) void f_025F9448_orig(Cpu*) {}
}

extern "C" void hook_025F9448(Cpu* c) {
    const uint32_t self = c->r[3];
    f_025F9448_orig(c);
    const game_lang::Start s = game_lang::current();
    if (!s.pack || !self) return;
    st32(self + 0x10, (uint32_t)s.region);
    st32(self + 0x14, (uint32_t)s.language);

    const uint32_t save = ld32(GD(0x101F84DC));
    if (save) st8(save + 0x12F0 + 4, (uint8_t)game_lang::options_language(s.language));
    static bool logged = false;
    if (!logged) {
        logged = true;
        LOG("[config] language source active: %s (%s), %s; the game is told region %d, language %d",
            game_lang::name(s.language), game_lang::region_name(s.region), s.pack->file.c_str(), s.region, s.language);
    }
}

namespace {
thread_local uint32_t t_name_message = 0;

bool german_europe() {
    const uint32_t setting = ld32(GD(0x101F4BAC));
    return setting && ld32(setting + 0x10) == (uint32_t)game_lang::kEurope && ld32(setting + 0x14) == 3;
}
}

extern "C" void site_025F8618(Cpu* c) { t_name_message = c->r[5]; }

extern "C" void site_025F8720(Cpu* c) {
    const uint32_t id = t_name_message;
    t_name_message = 0;
    if (id != 0xC8B && id != 0x1D21 && id != 0x31D7) return;
    if (!german_europe()) return;
    const uint32_t buf = c->r[30];
    const int32_t cap = (int32_t)ld32(c->r[1] + 0x18);
    int32_t len = 0;
    while (len < cap && ld8(buf + len)) len++;
    std::string name;
    for (int32_t i = 0; i < len; i++) name.push_back((char)ld8(buf + i));
    const char* suffix = game_lang::german_genitive_suffix(name);
    int32_t n = (int32_t)strlen(suffix);
    if (n > cap - len - 1) n = cap - len - 1;
    if (n <= 0) return;
    for (int32_t i = 0; i < n; i++) st8(buf + len + i, (uint8_t)suffix[i]);
    st8(buf + len + n, 0);
}

extern "C" void site_025FC668(Cpu* c) {
    const uint32_t self = c->r[25];
    const uint32_t obj = self ? ld32(self) : 0;
    const uint32_t id = obj ? ld32(obj + 0x11C) : 0;
    if (id != 0xC8B && id != 0x1D21 && id != 0x31D7) return;
    if (!german_europe()) return;
    const uint32_t buf = ld32(c->r[1] + 0x10);
    const int32_t cap = (int32_t)ld32(c->r[1] + 0x18);
    if (!buf || cap <= 0) return;
    int32_t len = 0;
    while (len < cap && ld16(buf + len * 2)) len++;
    std::string last;
    if (len > 0 && ld16(buf + (len - 1) * 2) < 0x80) last.push_back((char)ld16(buf + (len - 1) * 2));
    else if (len > 0) last.push_back('?');
    const char* suffix = game_lang::german_genitive_suffix(last);
    int32_t n = (int32_t)strlen(suffix);
    if (n > cap - len - 1) n = cap - len - 1;
    if (n <= 0) return;
    for (int32_t i = 0; i < n; i++) st16(buf + (len + i) * 2, (uint16_t)(uint8_t)suffix[i]);
    st16(buf + (len + n) * 2, 0);
}
