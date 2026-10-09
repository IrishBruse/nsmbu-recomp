#pragma once

namespace mods {

double game_time();

void mouse_init(void* tv_window);
void mouse_release();
bool mouse_captured();
void update_gyro_mouse();

}
