
#pragma once
#include <cstdint>

namespace audio {

constexpr int kRate = 48000;

void init();
void push(const int16_t* stereo, int frames);
int buffered_frames();
int target_frames();
void stats(uint64_t& underrun, uint64_t& dropped);
void flush();

}
