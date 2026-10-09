

#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace screenshot {

void request();

bool key_down(int code);

void poll_controller(const float* values);

bool take(uint64_t frame, std::string& tv, std::string& gamepad);
bool gamepad_too();
void set_gamepad_too(bool on);

void write_async(const std::string& path, uint32_t width, uint32_t height, size_t stride, const uint8_t* pixels,
                 std::shared_ptr<const void> owner, uint64_t frame, bool tv, bool bgra = false);

std::string dir();

void finish();
std::string last_file();

bool write_png(const std::string& path, uint32_t width, uint32_t height, size_t stride, const uint8_t* rgba, bool bgra = false);

}
