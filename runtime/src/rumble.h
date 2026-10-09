

#pragma once
#include <cstdint>

namespace rumble {

class Motor {
public:
    static constexpr uint32_t kBitUs = 8333;
    static constexpr uint32_t kMaxBits = 120;
    static constexpr uint64_t kProStaleUs = 500000;

    void gamepad_pattern(const uint8_t* bits, uint32_t nbits, uint64_t now);
    void gamepad_stop();
    void pro_motor(bool on, uint64_t now);
    void reset();

    float level(uint64_t now, uint64_t window) const;
    bool active(uint64_t now) const;

private:
    uint8_t pattern_[kMaxBits / 8] = {};
    uint32_t nbits_ = 0;
    uint64_t start_ = 0;
    bool pro_on_ = false;
    uint64_t pro_time_ = 0;
};

void gamepad_pattern(uint32_t chan, const uint8_t* bits, uint32_t nbits);
void gamepad_stop(uint32_t chan);
void pro_motor(uint32_t chan, bool on);
void reset();

bool enabled();
void set_enabled(bool on);
bool env_override();

float host_level(uint64_t window_us);

bool log_enabled();

}
