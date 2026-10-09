

#pragma once

namespace perf_hint {
void add_current_thread();
void frame_done();

bool fps60_chosen();
void set_fps60_chosen(bool on);
}
