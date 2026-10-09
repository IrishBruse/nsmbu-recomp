#pragma once
#include <cstddef>
#include <string_view>
namespace crash_context {
using Output = void (*)(int, const char*, size_t);
void initialize();
void refresh();
void note(int fd, Output out);
void redact(int fd, std::string_view text, Output out);
}
