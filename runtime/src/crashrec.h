

#pragma once
#include <string>

#include "input.h"

namespace crashrec {

constexpr int kAutoSlots = 3;
constexpr int kAutoBase = 100;

bool enabled();
void set_enabled(bool on);
int interval_seconds();

void service();

void on_auto_saved(int n);

input::PadState read(int pad);

struct AutoInfo {
    bool used = false;
    std::string when, area;
};
AutoInfo auto_info(int n);
void request_load(int n);

void crash_note(int fd, void (*out)(int, const char*, size_t));

}
