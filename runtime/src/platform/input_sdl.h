#pragma once
#include <SDL3/SDL.h>
namespace input {

void handle_event(const SDL_Event& event);
void update();
void set_prompt_window(SDL_Window* window);

bool text_prompt_active();

bool touch_from_controller(SDL_TouchID id);
}
