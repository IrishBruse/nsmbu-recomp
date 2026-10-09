

#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "input.h"

namespace input_map {

enum Action : int {
    kA, kB, kX, kY, kL, kR, kZL, kZR, kPlus, kMinus, kHome,
    kDUp, kDDown, kDLeft, kDRight, kStickLClick, kStickRClick,
    kLUp, kLDown, kLLeft, kLRight,
    kRUp, kRDown, kRLeft, kRRight,
    kScreenshot,
    kActionCount
};
const char* action_id(int a);
const char* action_label(int a);
uint32_t action_bit(int a);
int action_from_id(const std::string& id);

enum Pad : int {
    kPadNone,
    kPadA, kPadB, kPadX, kPadY, kPadLB, kPadRB, kPadLT, kPadRT,
    kPadMenu, kPadOptions, kPadHome, kPadL3, kPadR3,
    kPadDUp, kPadDDown, kPadDLeft, kPadDRight,
    kPadLSUp, kPadLSDown, kPadLSLeft, kPadLSRight,
    kPadRSUp, kPadRSDown, kPadRSLeft, kPadRSRight,
    kPadCount
};
const char* pad_id(int p);
const char* pad_label(int p);
int pad_from_id(const std::string& id);

constexpr int kNoKey = -1;
constexpr int kKeysPerAction = 2;
std::string key_id(int code);
std::string key_label(int code);
int key_from_id(const std::string& id);

const char* reserved_key(int code);

struct Mapping {
    std::array<std::array<int, kKeysPerAction>, kActionCount> keys;
    std::array<int, kActionCount> pad;
    float deadzone = 0.0f;
    bool invert_camera_y = false;
    static Mapping defaults();
    bool operator==(const Mapping&) const = default;
};

enum class FaceLayout { kPosition, kLabels, kCustom };
FaceLayout face_layout(const Mapping& m);
void apply_face_layout(Mapping& m, FaceLayout layout);
const char* face_layout_label(FaceLayout l);

std::vector<int> key_users(const Mapping& m, int code, int except = -1);
std::vector<int> pad_users(const Mapping& m, int pad, int except = -1);

bool has_conflict(const Mapping& m, int a);

int conflict_count(const Mapping& m);

std::string key_short_label(int code);
const char* pad_short_label(int p);

input::PadState keyboard_state(const Mapping& m, const bool keys[256]);
input::PadState controller_state(const Mapping& m, const float values[kPadCount]);

std::string to_json(const Mapping& m);
bool from_json(const std::string& text, Mapping& out, std::string* error = nullptr);
bool load_file(const std::string& path, Mapping& out, std::string* error = nullptr);
bool save_file(const std::string& path, const Mapping& m);
std::string default_path();

void load_startup();
Mapping current();
void set_current(const Mapping& m, bool save = true);
uint32_t generation();

}
