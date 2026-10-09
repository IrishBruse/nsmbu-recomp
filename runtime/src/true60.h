
#pragma once
#include <cstdint>

struct Cpu;

namespace true60 {
bool enabled();
void set_enabled(bool v);
float dt();
uint32_t exec_proc();
bool half_pass();
int32_t split(int32_t v);
void new_pass();
void pass_begin(bool full);
bool preview();
void camera_draw_preview(bool begin);
uint64_t pass();
bool runs_60(uint32_t proc);
bool drawing_60();
void force_draw_60(bool on);
uint32_t link();
uint64_t link_steps();
bool state_loaded();
void ss_reset();

void ratio_begin(Cpu* c, int r);
void ratio_end(Cpu* c, int r);
void ratio_arg(Cpu* c, int r);
float set_dt(float dt);

enum Group { kGrpLoco, kGrpCamera, kGrpSword, kGrpItems, kGrpSwim, kGrpSail, kGrpBk, kGrpMo2, kGrpCc, kGrpKi, kNumGroups };
bool group_on(Group g);

int link_proc_mode(uint32_t proc_id);
}
