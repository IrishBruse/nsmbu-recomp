

#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "mods/guest_identity.h"

namespace pstate {

constexpr int kFormatVersion = 1;
constexpr size_t kMaxFileSize = 64 * 1024;
constexpr size_t kMaxLine = 8192;
constexpr const char* kExtension = "wwstate";

constexpr size_t kSaveDataSize = 0xA94;
constexpr size_t kSaveDataUsed = 0x768;
constexpr size_t kSaveChecksumAt = 0xA8C;
constexpr size_t kHdPlayerSize = 16, kHdStatusSize = 4, kHdEventSize = 20, kHdMapSize = 220;

struct State {

    int format = kFormatVersion;
    std::string title_id;
    uint32_t title_version = 0;
    std::string game_hash;
    std::string runtime;
    std::string created;
    int controller = 0;
    int file_slot = 0;
    std::string player_name;
    std::vector<guestmods::ModIdentity> guest_mods;

    std::string stage;
    int start_point = 0, start_room = 0, layer = -1;
    int room = 0;
    float pos[3] = {0, 0, 0};
    int angle_y = 0;
    int link_proc = -1;
    bool on_ship = false;
    bool has_ship = false;
    float ship_pos[3] = {0, 0, 0};
    int ship_angle_y = 0;
    float time_of_day = 0;
    int date = 0;

    std::vector<uint8_t> savedata, hd_player, hd_status, hd_event, hd_map;
};

uint32_t crc32(const void* data, size_t n, uint32_t crc = 0);

void save_checksum(const uint8_t* block, uint32_t& sum, uint32_t& complement);

void seal_savedata(std::vector<uint8_t>& block);
bool savedata_checksum_ok(const std::vector<uint8_t>& block);

std::string write(const State& s, std::string& why);

bool read(const std::string& text, State& out, std::string& why);

bool blob_check(const std::string& text, std::string& why);

}
