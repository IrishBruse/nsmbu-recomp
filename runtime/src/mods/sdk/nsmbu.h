#pragma once

struct lua_State;
struct Cpu;

namespace mods::luau_mod {
struct Instance;
}

namespace mods::luau_sdk {

void install(lua_State* L, luau_mod::Instance* inst);
void set_cpu(Cpu* c);
Cpu* cpu();

}
