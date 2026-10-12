#pragma once

struct lua_State;
struct Cpu;

namespace mods::luau_game {

void install(lua_State* L);
void set_cpu(Cpu* c);

}
