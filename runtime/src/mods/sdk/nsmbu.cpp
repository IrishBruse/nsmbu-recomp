#include "nsmbu.h"
#include "mods/luau_runtime.h"
#include "mods/luau_guest.h"
#include "mods/luau_press.h"
#include "guest_addr.h"
#include "input.h"
#include "runtime.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "lua.h"
#include "lualib.h"

namespace mods::luau_sdk {
namespace {
using luau_mod::Instance;
using json::Value;

Instance* self(lua_State* L) {
    return static_cast<Instance*>(lua_touserdata(L, lua_upvalueindex(1)));
}

int fail(lua_State* L, const char* msg) {
    lua_pushnil(L);
    lua_pushstring(L, msg);
    return 2;
}

bool push_loaded(lua_State* L, const char* name) {
    lua_getglobal(L, "package");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_pushnil(L);
        return false;
    }
    lua_getfield(L, -1, "loaded");
    lua_remove(L, -2);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_pushnil(L);
        return false;
    }
    lua_getfield(L, -1, name);
    lua_remove(L, -2);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_pushnil(L);
        return false;
    }
    return true;
}

void set_loaded(lua_State* L, const char* name) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "loaded");
    lua_pushvalue(L, -3);
    lua_setfield(L, -2, name);
    lua_pop(L, 3);
}

void refresh_package(Instance& inst) {
    lua_State* L = static_cast<lua_State*>(inst.state);
    if (!L) return;
    if (!push_loaded(L, "nsmbu")) {
        lua_pop(L, 1);
        return;
    }
    lua_getfield(L, -1, "package");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, "package");
    }
    lua_pushlstring(L, inst.id.data(), inst.id.size());
    lua_setfield(L, -2, "id");
    lua_pushlstring(L, inst.version.data(), inst.version.size());
    lua_setfield(L, -2, "version");
    lua_pushlstring(L, inst.path.data(), inst.path.size());
    lua_setfield(L, -2, "path");
    lua_pop(L, 2);
}

void bind(lua_State* L, Instance* inst, const char* name, lua_CFunction fn) {
    lua_pushlightuserdata(L, inst);
    lua_pushcclosure(L, fn, name, 1);
    lua_setfield(L, -2, name);
}

void bind_guest(lua_State* L, const char* name, lua_CFunction fn) {
    lua_pushcfunction(L, fn, name);
    lua_setfield(L, -2, name);
}

void set_empty_module(lua_State* L, const char* name) {
    lua_newtable(L);
    set_loaded(L, name);
}

void attach_loaded(lua_State* L, const char* name, const char* field) {
    if (push_loaded(L, name)) lua_setfield(L, -2, field);
    else lua_pop(L, 1);
}

int listen_event(lua_State* L, const char* event) {
    Instance* inst = self(L);
    if (!lua_isfunction(L, 1)) return fail(L, "callback must be a function");
    int ref = lua_ref(L, 1);
    int id = inst->next_listener++;
    inst->listeners.push_back({id, ref, event});
    lua_pushinteger(L, id);
    return 1;
}

int l_on_logic_step(lua_State* L) { return listen_event(L, "logic_step"); }

int l_on_config_changed(lua_State* L) { return listen_event(L, "config_changed"); }

int l_on_unload(lua_State* L) { return listen_event(L, "unload"); }

int l_unlisten(lua_State* L) {
    Instance* inst = self(L);
    int id = (int)luaL_checkinteger(L, 1);
    for (auto it = inst->listeners.begin(); it != inst->listeners.end(); ++it) {
        if (it->id != id) continue;
        lua_unref(L, it->ref);
        inst->listeners.erase(it);
        lua_pushboolean(L, 1);
        return 1;
    }
    return fail(L, "unknown listener");
}

int l_log(lua_State* L) {
    Instance* inst = self(L);
    if (!lua_isstring(L, 1)) return fail(L, "message must be a string");
    fprintf(stderr, "[mod:%s] %s\n", inst->id.c_str(), lua_tostring(L, 1));
    lua_pushboolean(L, 1);
    return 1;
}

int l_log_int(lua_State* L) {
    Instance* inst = self(L);
    if (!lua_isstring(L, 1)) return fail(L, "label must be a string");
    if (!lua_isnumber(L, 2)) return fail(L, "value must be a number");
    fprintf(stderr, "[mod:%s] %s %lld\n", inst->id.c_str(), lua_tostring(L, 1),
            (long long)lua_tointeger(L, 2));
    lua_pushboolean(L, 1);
    return 1;
}

int l_log_hex(lua_State* L) {
    Instance* inst = self(L);
    if (!lua_isstring(L, 1)) return fail(L, "label must be a string");
    if (!lua_isnumber(L, 2)) return fail(L, "value must be a number");
    fprintf(stderr, "[mod:%s] %s %08X\n", inst->id.c_str(), lua_tostring(L, 1),
            (unsigned)(uint32_t)lua_tointeger(L, 2));
    lua_pushboolean(L, 1);
    return 1;
}

int l_log_float(lua_State* L) {
    Instance* inst = self(L);
    if (!lua_isstring(L, 1)) return fail(L, "label must be a string");
    if (!lua_isnumber(L, 2)) return fail(L, "value must be a number");
    fprintf(stderr, "[mod:%s] %s %.17g\n", inst->id.c_str(), lua_tostring(L, 1),
            (double)lua_tonumber(L, 2));
    lua_pushboolean(L, 1);
    return 1;
}

int l_status(lua_State* L) {
    Instance* inst = self(L);
    if (!lua_isstring(L, 1)) return fail(L, "message must be a string");
    size_t len = 0;
    const char* msg = lua_tolstring(L, 1, &len);
    if (len > 1024) len = 1024;
    inst->status_line.assign(msg, len);
    lua_pushboolean(L, 1);
    return 1;
}

int l_logic_step(lua_State* L) {
    Instance* inst = self(L);
    lua_pushnumber(L, (lua_Number)inst->step);
    return 1;
}

int l_logic_dt(lua_State* L) {
    lua_pushnumber(L, 1.0 / 60.0);
    return 1;
}

int l_config_string(lua_State* L) {
    Instance* inst = self(L);
    const char* id = luaL_checkstring(L, 1);
    if (!lua_isstring(L, 2)) return fail(L, "fallback must be a string");
    const auto& v = inst->config.get(id);
    if (v.type == Value::Null || v.type != Value::String) {
        lua_pushvalue(L, 2);
        return 1;
    }
    lua_pushlstring(L, v.text.data(), v.text.size());
    return 1;
}

int l_config_number(lua_State* L) {
    Instance* inst = self(L);
    const char* id = luaL_checkstring(L, 1);
    if (!lua_isnumber(L, 2)) return fail(L, "fallback must be a number");
    const auto& v = inst->config.get(id);
    if (v.type == Value::Null || v.type != Value::Number) {
        lua_pushvalue(L, 2);
        return 1;
    }
    lua_pushnumber(L, v.number);
    return 1;
}

int l_config_bool(lua_State* L) {
    Instance* inst = self(L);
    const char* id = luaL_checkstring(L, 1);
    if (!lua_isboolean(L, 2)) return fail(L, "fallback must be a boolean");
    const auto& v = inst->config.get(id);
    if (v.type == Value::Null || v.type != Value::Bool) {
        lua_pushvalue(L, 2);
        return 1;
    }
    lua_pushboolean(L, v.boolean);
    return 1;
}

uint32_t addr_arg(lua_State* L, int idx) { return (uint32_t)(int64_t)luaL_checknumber(L, idx); }

int guest_read_u8(lua_State* L) {
    std::string err;
    uint8_t v;
    if (!luau_guest::read_u8(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushinteger(L, v);
    return 1;
}
int guest_read_s8(lua_State* L) {
    std::string err;
    int8_t v;
    if (!luau_guest::read_s8(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushinteger(L, v);
    return 1;
}
int guest_read_u16(lua_State* L) {
    std::string err;
    uint16_t v;
    if (!luau_guest::read_u16(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushinteger(L, v);
    return 1;
}
int guest_read_s16(lua_State* L) {
    std::string err;
    int16_t v;
    if (!luau_guest::read_s16(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushinteger(L, v);
    return 1;
}
int guest_read_u32(lua_State* L) {
    std::string err;
    uint32_t v;
    if (!luau_guest::read_u32(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)v);
    return 1;
}
int guest_read_s32(lua_State* L) {
    std::string err;
    int32_t v;
    if (!luau_guest::read_s32(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)v);
    return 1;
}
int guest_read_f32(lua_State* L) {
    std::string err;
    float v;
    if (!luau_guest::read_f32(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushnumber(L, v);
    return 1;
}
int guest_read_f64(lua_State* L) {
    std::string err;
    double v;
    if (!luau_guest::read_f64(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushnumber(L, v);
    return 1;
}
int guest_read(lua_State* L) {
    uint32_t addr = addr_arg(L, 1);
    size_t size = (size_t)luaL_checkinteger(L, 2);
    std::string err, out;
    if (!luau_guest::read_bytes(addr, size, out, err)) return fail(L, err.c_str());
    lua_pushlstring(L, out.data(), out.size());
    return 1;
}

int guest_write_u8(lua_State* L) {
    std::string err;
    if (!luau_guest::write_u8(addr_arg(L, 1), (uint8_t)(uint32_t)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write_s8(lua_State* L) {
    std::string err;
    if (!luau_guest::write_s8(addr_arg(L, 1), (int8_t)(int32_t)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write_u16(lua_State* L) {
    std::string err;
    if (!luau_guest::write_u16(addr_arg(L, 1), (uint16_t)(uint32_t)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write_s16(lua_State* L) {
    std::string err;
    if (!luau_guest::write_s16(addr_arg(L, 1), (int16_t)(int32_t)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write_u32(lua_State* L) {
    std::string err;
    if (!luau_guest::write_u32(addr_arg(L, 1), (uint32_t)(int64_t)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write_s32(lua_State* L) {
    std::string err;
    if (!luau_guest::write_s32(addr_arg(L, 1), (int32_t)(int64_t)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write_f32(lua_State* L) {
    std::string err;
    if (!luau_guest::write_f32(addr_arg(L, 1), (float)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write_f64(lua_State* L) {
    std::string err;
    if (!luau_guest::write_f64(addr_arg(L, 1), (double)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write(lua_State* L) {
    uint32_t addr = addr_arg(L, 1);
    size_t len = 0;
    const char* data = luaL_checklstring(L, 2, &len);
    std::string err;
    if (!luau_guest::write_bytes(addr, data, len, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

bool button_bit(const char* name, uint32_t& bit) {
    static const struct { const char* name; uint32_t bit; } kTable[] = {
        {"A", input::kA},       {"B", input::kB},       {"X", input::kX},
        {"Y", input::kY},       {"L", input::kL},       {"R", input::kR},
        {"ZL", input::kZL},     {"ZR", input::kZR},     {"Plus", input::kPlus},
        {"Minus", input::kMinus}, {"Home", input::kHome},
        {"Up", input::kUp},     {"Down", input::kDown},
        {"Left", input::kLeft}, {"Right", input::kRight},
        {"StickL", input::kStickL}, {"StickR", input::kStickR},
    };
    for (const auto& entry : kTable) {
        if (std::strcmp(entry.name, name) != 0) continue;
        bit = entry.bit;
        return true;
    }
    return false;
}

bool button_mask(lua_State* L, int arg, uint32_t& mask) {
    if (lua_type(L, arg) == LUA_TSTRING) {
        uint32_t bit = 0;
        if (!button_bit(lua_tostring(L, arg), bit)) return false;
        mask |= bit;
        return true;
    }
    if (lua_type(L, arg) != LUA_TTABLE) return false;
    lua_pushnil(L);
    while (lua_next(L, arg) != 0) {
        uint32_t bit = 0;
        if (lua_type(L, -1) != LUA_TSTRING || !button_bit(lua_tostring(L, -1), bit)) {
            lua_pop(L, 2);
            return false;
        }
        mask |= bit;
        lua_pop(L, 1);
    }
    return true;
}

int l_input_press(lua_State* L) {
    Instance* inst = self(L);
    uint32_t mask = 0;
    if (!button_mask(L, 1, mask)) return fail(L, "bad type");
    if (!lua_isnumber(L, 2)) return fail(L, "steps must be a number");
    double steps = lua_tonumber(L, 2);
    if (steps < 1) steps = 1;
    if (steps > 600) steps = 600;
    luau_press::press(inst, mask, (uint64_t)steps, inst->step);
    lua_pushboolean(L, 1);
    return 1;
}

const uint32_t kCourseTimerSingleton = GD(0x101D15F4);
const uint32_t kFieldGameSingleton = GD(0x101D1604);
const uint32_t kPlayerMgrSingleton = GD(0x101E6994);
const uint32_t kActorMgrSingleton = GD(0x101C30C8);
const uint32_t kProfileList = GD(0x101ED8DC);
const uint32_t kActorMgrCreateLater = GC(0x02009954);
const uint32_t kTypeInfoActor = GD(0x101E9CC0);
const uint32_t kCourseSelectInstance = GD(0x101CC96C);
const uint32_t kCourseTaskInstance = GD(0x101D15F0);
const uint32_t kSceneSingleton = GD(0x101E7B28);
const uint32_t kSetNextScene = GC(0x02994164);
const uint32_t kSetCourseStory = GC(0x029941D0);
const uint32_t kAppOpenFileSelect = GC(0x0297D5A8);
const uint32_t kAppConfirmFile = GC(0x0297EC3C);

constexpr uint32_t kSceneNowCreatorOff = 0x58u;
constexpr uint32_t kSceneFlowOff = 0x14u;
constexpr uint32_t kAppGateSelect = 0xDB1u;
constexpr uint32_t kAppGateStory = 0xDB2u;
constexpr uint32_t kCreatorBoot = 0x0202930Cu;
constexpr uint32_t kCreatorFlow = 0x0297B9F8u;
constexpr uint32_t kCreatorFileSelect = 0x0298AD3Cu;
constexpr uint32_t kCreatorTitle = 0x0202B51Cu;
constexpr uint32_t kCreatorMap = 0x020292B8u;
constexpr uint32_t kCreatorCourseSelect = 0x0202B6C4u;
constexpr uint32_t kClassIdTask = 0x00000002u;

constexpr uint32_t kActorMgrPtrCache = 0x6A20u;
constexpr uint32_t kActorIdxToPtr = 0x4u;
constexpr uint32_t kCacheArraySize = 0x0u;
constexpr uint32_t kCacheArray = 0x4u;
constexpr uint32_t kCacheLiveEnd = 0xCu;
constexpr uint32_t kActorId = 0x4u;
constexpr uint32_t kActorProfile = 0x8u;
constexpr uint32_t kActorIsMap = 0xDu;
constexpr uint32_t kActorIsActive = 0xEu;
constexpr uint32_t kActorDeleteFlag = 0xFu;
constexpr uint32_t kActorParam0 = 0x10u;
constexpr uint32_t kActorParam1 = 0x14u;
constexpr uint32_t kActorVptr = 0x4Cu;
constexpr uint32_t kDerivedCheckSlot = 0xCu;
constexpr uint32_t kTimerTime = 0x14u;
constexpr uint32_t kCourseCoinNum = 0x144u;
constexpr uint32_t kCurrentScore = 0x148u;
constexpr uint32_t kPlayerData0 = 0x24u;
constexpr uint32_t kPlayerDataSize = 0x48u;
constexpr uint32_t kEntryOff = 0x0u;
constexpr uint32_t kLifeOff = 0x4u;
constexpr uint32_t kModeOff = 0x8u;
constexpr uint32_t kCoinOff = 0x34u;
constexpr uint32_t kPlayerObjects = 0x20u;
constexpr uint32_t kPlayerNo = 0x54u;
constexpr uint32_t kLayer = 0x56u;
constexpr uint32_t kPosX = 0x6Cu;
constexpr uint32_t kPosY = 0x70u;
constexpr uint32_t kPosZ = 0x74u;
constexpr uint32_t kSpeedX = 0x78u;
constexpr uint32_t kSpeedY = 0x7Cu;
constexpr uint32_t kSpeedZ = 0x80u;
constexpr uint32_t kScaleX = 0x90u;
constexpr uint32_t kScaleY = 0x94u;
constexpr uint32_t kScaleZ = 0x98u;
constexpr uint32_t kActorMode = 0x500u;
constexpr uint32_t kNowBgCross = 0x1AC8u;
constexpr uint32_t kBgCrossIsFoot = 1u;

constexpr int kPlayerCount = 4;
constexpr uint32_t kActorArraySize = 520;
constexpr const char* kPlayerMeta = "nsmbu.player";
constexpr const char* kActorMeta = "nsmbu.level.actor";

thread_local Cpu* t_guest_cpu = nullptr;
bool g_actor_shape_warned = false;

bool guest_ok(uint32_t addr, size_t size, std::string& err) {
    return luau_guest::check(addr, size, err);
}

bool read_ptr(uint32_t addr, uint32_t& out, std::string& err) {
    uint32_t p = 0;
    if (!luau_guest::read_u32(addr, p, err)) return false;
    if (p == 0 || !guest_ok(p, 4, err)) {
        err = "not available";
        return false;
    }
    out = p;
    return true;
}

bool singleton(uint32_t addr, uint32_t& out, std::string& err) {
    return read_ptr(addr, out, err);
}

bool field_game(uint32_t& out, std::string& err) {
    return singleton(kFieldGameSingleton, out, err);
}

bool player_mgr(uint32_t& out, std::string& err) {
    return singleton(kPlayerMgrSingleton, out, err);
}

bool slot_addr(int index, uint32_t& out, std::string& err) {
    if (index < 0 || index >= kPlayerCount) {
        err = "bad player index";
        return false;
    }
    uint32_t field = 0;
    if (!field_game(field, err)) return false;
    out = field + kPlayerData0 + (uint32_t)index * kPlayerDataSize;
    return true;
}

bool actor_addr(int index, uint32_t& out, std::string& err) {
    if (index < 0 || index >= kPlayerCount) {
        err = "bad player index";
        return false;
    }
    uint32_t mgr = 0;
    if (!player_mgr(mgr, err)) return false;
    uint32_t actor = 0;
    if (!luau_guest::read_u32(mgr + kPlayerObjects + (uint32_t)index * 4u, actor, err)) return false;
    if (actor == 0 || !guest_ok(actor, 4, err)) {
        err = "no player actor";
        return false;
    }
    out = actor;
    return true;
}

bool player_entered(int index, bool& out, std::string& err) {
    uint32_t slot = 0;
    if (!slot_addr(index, slot, err)) return false;
    uint8_t entry = 0;
    if (!luau_guest::read_u8(slot + kEntryOff, entry, err)) return false;
    out = entry != 0;
    return true;
}

int player_index(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "index");
    if (!lua_isnumber(L, -1)) luaL_error(L, "player index missing");
    int index = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);
    return index;
}

void push_player(lua_State* L, int index) {
    lua_newtable(L);
    lua_pushinteger(L, index);
    lua_setfield(L, -2, "index");
    luaL_getmetatable(L, kPlayerMeta);
    lua_setmetatable(L, -2);
}

void push_vec3(lua_State* L, float x, float y, float z) {
    lua_newtable(L);
    lua_pushnumber(L, x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, z);
    lua_setfield(L, -2, "z");
}

bool read_xyz(lua_State* L, int arg, float& x, float& y, float& z, std::string& err) {
    if (lua_istable(L, arg)) {
        lua_getfield(L, arg, "x");
        if (!lua_isnumber(L, -1)) {
            lua_pop(L, 1);
            err = "bad type";
            return false;
        }
        x = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, arg, "y");
        if (!lua_isnumber(L, -1)) {
            lua_pop(L, 1);
            err = "bad type";
            return false;
        }
        y = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, arg, "z");
        if (!lua_isnumber(L, -1)) {
            lua_pop(L, 1);
            err = "bad type";
            return false;
        }
        z = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);
        return true;
    }
    if (!lua_isnumber(L, arg) || !lua_isnumber(L, arg + 1) || !lua_isnumber(L, arg + 2)) {
        err = "bad type";
        return false;
    }
    x = (float)lua_tonumber(L, arg);
    y = (float)lua_tonumber(L, arg + 1);
    z = (float)lua_tonumber(L, arg + 2);
    return true;
}

bool read_vec3(uint32_t base, uint32_t ox, uint32_t oy, uint32_t oz, float& x, float& y, float& z,
               std::string& err) {
    if (!luau_guest::read_f32(base + ox, x, err)) return false;
    if (!luau_guest::read_f32(base + oy, y, err)) return false;
    if (!luau_guest::read_f32(base + oz, z, err)) return false;
    return true;
}

bool write_vec3(uint32_t base, uint32_t ox, uint32_t oy, uint32_t oz, float x, float y, float z,
                std::string& err) {
    if (!luau_guest::write_f32(base + ox, x, err)) return false;
    if (!luau_guest::write_f32(base + oy, y, err)) return false;
    if (!luau_guest::write_f32(base + oz, z, err)) return false;
    return true;
}

int l_course_active(lua_State* L) {
    uint32_t timer = 0;
    std::string err;
    lua_pushboolean(L, singleton(kCourseTimerSingleton, timer, err) ? 1 : 0);
    return 1;
}

int l_course_time(lua_State* L) {
    uint32_t timer = 0;
    std::string err;
    if (!singleton(kCourseTimerSingleton, timer, err)) return fail(L, err.c_str());
    uint32_t units = 0;
    if (!luau_guest::read_u32(timer + kTimerTime, units, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)(units >> 12));
    return 1;
}

int l_course_set_time(lua_State* L) {
    if (!lua_isnumber(L, 1)) return fail(L, "bad type");
    double seconds = lua_tonumber(L, 1);
    if (seconds < 0) seconds = 0;
    if (seconds > 999) seconds = 999;
    uint32_t timer = 0;
    std::string err;
    if (!singleton(kCourseTimerSingleton, timer, err)) return fail(L, err.c_str());
    uint32_t units = ((uint32_t)seconds) << 12;
    if (!luau_guest::write_u32(timer + kTimerTime, units, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_course_add_time(lua_State* L) {
    if (!lua_isnumber(L, 1)) return fail(L, "bad type");
    double delta = lua_tonumber(L, 1);
    uint32_t timer = 0;
    std::string err;
    if (!singleton(kCourseTimerSingleton, timer, err)) return fail(L, err.c_str());
    uint32_t units = 0;
    if (!luau_guest::read_u32(timer + kTimerTime, units, err)) return fail(L, err.c_str());
    double seconds = (double)(units >> 12) + delta;
    if (seconds < 0) seconds = 0;
    if (seconds > 999) seconds = 999;
    units = ((uint32_t)seconds) << 12;
    if (!luau_guest::write_u32(timer + kTimerTime, units, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_course_coins(lua_State* L) {
    uint32_t field = 0;
    std::string err;
    if (!field_game(field, err)) return fail(L, err.c_str());
    int32_t coins = 0;
    if (!luau_guest::read_s32(field + kCourseCoinNum, coins, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)coins);
    return 1;
}

int l_course_set_coins(lua_State* L) {
    if (!lua_isnumber(L, 1)) return fail(L, "bad type");
    uint32_t field = 0;
    std::string err;
    if (!field_game(field, err)) return fail(L, err.c_str());
    int32_t coins = (int32_t)lua_tonumber(L, 1);
    if (!luau_guest::write_s32(field + kCourseCoinNum, coins, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_course_score(lua_State* L) {
    uint32_t field = 0;
    std::string err;
    if (!field_game(field, err)) return fail(L, err.c_str());
    uint32_t score = 0;
    if (!luau_guest::read_u32(field + kCurrentScore, score, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)score);
    return 1;
}

int l_course_set_score(lua_State* L) {
    if (!lua_isnumber(L, 1)) return fail(L, "bad type");
    uint32_t field = 0;
    std::string err;
    if (!field_game(field, err)) return fail(L, err.c_str());
    uint32_t score = (uint32_t)lua_tonumber(L, 1);
    if (!luau_guest::write_u32(field + kCurrentScore, score, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_player(lua_State* L) {
    if (!lua_isnumber(L, 1)) return fail(L, "bad type");
    int index = (int)lua_tointeger(L, 1);
    if (index < 0 || index >= kPlayerCount) return fail(L, "bad player index");
    uint32_t field = 0;
    std::string err;
    if (!field_game(field, err)) return fail(L, err.c_str());
    push_player(L, index);
    return 1;
}

int l_players(lua_State* L) {
    lua_newtable(L);
    std::string err;
    for (int i = 0; i < kPlayerCount; ++i) {
        bool entered = false;
        if (!player_entered(i, entered, err)) continue;
        if (!entered) continue;
        push_player(L, i);
        lua_rawseti(L, -2, (int)lua_objlen(L, -2) + 1);
    }
    return 1;
}

int l_player_entered(lua_State* L) {
    int index = player_index(L);
    bool entered = false;
    std::string err;
    if (!player_entered(index, entered, err)) return fail(L, err.c_str());
    lua_pushboolean(L, entered ? 1 : 0);
    return 1;
}

int l_player_lives(lua_State* L) {
    int index = player_index(L);
    bool entered = false;
    std::string err;
    if (!player_entered(index, entered, err)) return fail(L, err.c_str());
    if (!entered) return fail(L, "not entered");
    uint32_t slot = 0;
    if (!slot_addr(index, slot, err)) return fail(L, err.c_str());
    int32_t lives = 0;
    if (!luau_guest::read_s32(slot + kLifeOff, lives, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)lives);
    return 1;
}

int l_player_set_lives(lua_State* L) {
    int index = player_index(L);
    if (!lua_isnumber(L, 2)) return fail(L, "bad type");
    bool entered = false;
    std::string err;
    if (!player_entered(index, entered, err)) return fail(L, err.c_str());
    if (!entered) return fail(L, "not entered");
    uint32_t slot = 0;
    if (!slot_addr(index, slot, err)) return fail(L, err.c_str());
    int32_t lives = (int32_t)lua_tonumber(L, 2);
    if (!luau_guest::write_s32(slot + kLifeOff, lives, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_player_mode(lua_State* L) {
    int index = player_index(L);
    bool entered = false;
    std::string err;
    if (!player_entered(index, entered, err)) return fail(L, err.c_str());
    if (!entered) return fail(L, "not entered");
    uint32_t slot = 0;
    if (!slot_addr(index, slot, err)) return fail(L, err.c_str());
    int32_t mode = 0;
    if (!luau_guest::read_s32(slot + kModeOff, mode, err)) return fail(L, err.c_str());
    uint32_t actor = 0;
    if (actor_addr(index, actor, err)) {
        int32_t actor_mode = 0;
        if (luau_guest::read_s32(actor + kActorMode, actor_mode, err) && actor_mode >= 0 &&
            actor_mode <= 8)
            mode = actor_mode;
    }
    if (mode < 0 || mode > 8) return fail(L, "bad mode");
    lua_pushnumber(L, (lua_Number)mode);
    return 1;
}

int l_player_set_mode(lua_State* L) {
    int index = player_index(L);
    if (!lua_isnumber(L, 2)) return fail(L, "bad type");
    int32_t mode = (int32_t)lua_tonumber(L, 2);
    if (mode < 0 || mode > 8) return fail(L, "bad mode");
    bool entered = false;
    std::string err;
    if (!player_entered(index, entered, err)) return fail(L, err.c_str());
    if (!entered) return fail(L, "not entered");
    uint32_t slot = 0;
    if (!slot_addr(index, slot, err)) return fail(L, err.c_str());
    if (!luau_guest::write_s32(slot + kModeOff, mode, err)) return fail(L, err.c_str());
    uint32_t actor = 0;
    if (actor_addr(index, actor, err)) {
        if (!luau_guest::write_s32(actor + kActorMode, mode, err)) return fail(L, err.c_str());
    }
    lua_pushboolean(L, 1);
    return 1;
}

int l_player_coins(lua_State* L) {
    int index = player_index(L);
    bool entered = false;
    std::string err;
    if (!player_entered(index, entered, err)) return fail(L, err.c_str());
    if (!entered) return fail(L, "not entered");
    uint32_t slot = 0;
    if (!slot_addr(index, slot, err)) return fail(L, err.c_str());
    int32_t coins = 0;
    if (!luau_guest::read_s32(slot + kCoinOff, coins, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)coins);
    return 1;
}

int l_player_set_coins(lua_State* L) {
    int index = player_index(L);
    if (!lua_isnumber(L, 2)) return fail(L, "bad type");
    bool entered = false;
    std::string err;
    if (!player_entered(index, entered, err)) return fail(L, err.c_str());
    if (!entered) return fail(L, "not entered");
    uint32_t slot = 0;
    if (!slot_addr(index, slot, err)) return fail(L, err.c_str());
    int32_t coins = (int32_t)lua_tonumber(L, 2);
    if (!luau_guest::write_s32(slot + kCoinOff, coins, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_player_speed_y(lua_State* L) {
    int index = player_index(L);
    uint32_t actor = 0;
    std::string err;
    if (!actor_addr(index, actor, err)) return fail(L, err.c_str());
    float speed = 0;
    if (!luau_guest::read_f32(actor + kSpeedY, speed, err)) return fail(L, err.c_str());
    lua_pushnumber(L, speed);
    return 1;
}

int l_player_set_speed_y(lua_State* L) {
    int index = player_index(L);
    if (!lua_isnumber(L, 2)) return fail(L, "bad type");
    uint32_t actor = 0;
    std::string err;
    if (!actor_addr(index, actor, err)) return fail(L, err.c_str());
    float speed = (float)lua_tonumber(L, 2);
    if (!luau_guest::write_f32(actor + kSpeedY, speed, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_player_pos(lua_State* L) {
    int index = player_index(L);
    uint32_t actor = 0;
    std::string err;
    if (!actor_addr(index, actor, err)) return fail(L, err.c_str());
    float x = 0, y = 0, z = 0;
    if (!read_vec3(actor, kPosX, kPosY, kPosZ, x, y, z, err)) return fail(L, err.c_str());
    push_vec3(L, x, y, z);
    return 1;
}

int l_player_set_pos(lua_State* L) {
    int index = player_index(L);
    float x = 0, y = 0, z = 0;
    std::string err;
    if (!read_xyz(L, 2, x, y, z, err)) return fail(L, err.c_str());
    uint32_t actor = 0;
    if (!actor_addr(index, actor, err)) return fail(L, err.c_str());
    if (!write_vec3(actor, kPosX, kPosY, kPosZ, x, y, z, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_player_speed(lua_State* L) {
    int index = player_index(L);
    uint32_t actor = 0;
    std::string err;
    if (!actor_addr(index, actor, err)) return fail(L, err.c_str());
    float x = 0, y = 0, z = 0;
    if (!read_vec3(actor, kSpeedX, kSpeedY, kSpeedZ, x, y, z, err)) return fail(L, err.c_str());
    push_vec3(L, x, y, z);
    return 1;
}

int l_player_set_speed(lua_State* L) {
    int index = player_index(L);
    float x = 0, y = 0, z = 0;
    std::string err;
    if (!read_xyz(L, 2, x, y, z, err)) return fail(L, err.c_str());
    uint32_t actor = 0;
    if (!actor_addr(index, actor, err)) return fail(L, err.c_str());
    if (!write_vec3(actor, kSpeedX, kSpeedY, kSpeedZ, x, y, z, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_player_grounded(lua_State* L) {
    int index = player_index(L);
    uint32_t actor = 0;
    std::string err;
    if (!actor_addr(index, actor, err)) return fail(L, err.c_str());
    uint32_t cross = 0;
    if (!luau_guest::read_u32(actor + kNowBgCross, cross, err)) return fail(L, err.c_str());
    lua_pushboolean(L, (cross & kBgCrossIsFoot) ? 1 : 0);
    return 1;
}

int l_player_actor(lua_State* L) {
    int index = player_index(L);
    uint32_t actor = 0;
    std::string err;
    if (!actor_addr(index, actor, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)actor);
    return 1;
}

bool actor_mgr(uint32_t& out, std::string& err) {
    return read_ptr(kActorMgrSingleton, out, err);
}

bool live_actor_range(uint32_t& begin, uint32_t& end, std::string& err) {
    uint32_t mgr = 0;
    if (!actor_mgr(mgr, err)) return false;
    uint32_t cache = mgr + kActorMgrPtrCache;
    uint32_t array = 0, size = 0, live = 0;
    if (!luau_guest::read_u32(cache + kCacheArray, array, err)) return false;
    if (!luau_guest::read_u32(cache + kCacheArraySize, size, err)) return false;
    if (!luau_guest::read_u32(cache + kCacheLiveEnd, live, err)) return false;
    if (size != kActorArraySize || array == 0 || !guest_ok(array, size * 4, err)) {
        if (!g_actor_shape_warned) {
            g_actor_shape_warned = true;
            fprintf(stderr, "[mods] nsmbu.level actor cache mismatch: mgr=%08x array=%08x size=%u live=%08x\n",
                    mgr, array, size, live);
        }
        err = "actor array not ready";
        return false;
    }
    if (live < array || live > array + size * 4 || ((live - array) & 3)) {
        err = "actor array not ready";
        return false;
    }
    begin = array;
    end = live;
    return true;
}

void push_actor(lua_State* L, uint32_t addr) {
    lua_newtable(L);
    lua_pushnumber(L, (lua_Number)addr);
    lua_setfield(L, -2, "addr");
    luaL_getmetatable(L, kActorMeta);
    lua_setmetatable(L, -2);
}

bool handle_addr(lua_State* L, uint32_t& out, std::string& err) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "addr");
    if (!lua_isnumber(L, -1)) {
        lua_pop(L, 1);
        err = "bad actor handle";
        return false;
    }
    out = (uint32_t)lua_tonumber(L, -1);
    lua_pop(L, 1);
    if (!guest_ok(out, kActorVptr + 4, err)) {
        err = "actor gone";
        return false;
    }
    return true;
}

bool actor_profile_id(uint32_t actor, uint32_t& out, std::string& err) {
    uint32_t profile = 0;
    if (!luau_guest::read_u32(actor + kActorProfile, profile, err)) return false;
    if (profile == 0 || !guest_ok(profile, 0x14, err)) {
        err = "actor gone";
        return false;
    }
    if (!luau_guest::read_u32(profile + kActorId, out, err)) return false;
    return true;
}

bool actor_is_actor(uint32_t actor, bool& out, std::string& err) {
    if (!t_guest_cpu) {
        err = "no guest cpu";
        return false;
    }
    uint32_t vt = 0;
    if (!luau_guest::read_u32(actor + kActorVptr, vt, err)) return false;
    if (vt == 0 || !guest_ok(vt, kDerivedCheckSlot + 4, err)) {
        out = false;
        return true;
    }
    uint32_t fn = 0;
    if (!luau_guest::read_u32(vt + kDerivedCheckSlot, fn, err)) return false;
    if (fn == 0 || !guest_code_valid(fn)) {
        out = false;
        return true;
    }
    out = guest_call(t_guest_cpu, fn, {actor, kTypeInfoActor}) != 0;
    return true;
}

int l_level_actors(lua_State* L) {
    uint32_t begin = 0, end = 0;
    std::string err;
    if (!live_actor_range(begin, end, err)) return fail(L, err.c_str());
    lua_newtable(L);
    int n = 0;
    for (uint32_t slot = begin; slot < end; slot += kActorIdxToPtr) {
        uint32_t actor = 0;
        if (!luau_guest::read_u32(slot, actor, err) || actor == 0 ||
            !guest_ok(actor, kActorVptr + 4, err))
            continue;
        push_actor(L, actor);
        lua_rawseti(L, -2, ++n);
    }
    return 1;
}

int l_level_count(lua_State* L) {
    uint32_t begin = 0, end = 0;
    std::string err;
    if (!live_actor_range(begin, end, err)) return fail(L, err.c_str());
    bool filter = lua_isnumber(L, 1) ? true : false;
    uint32_t want = filter ? (uint32_t)lua_tonumber(L, 1) : 0;
    int count = 0;
    for (uint32_t slot = begin; slot < end; slot += kActorIdxToPtr) {
        uint32_t actor = 0;
        if (!luau_guest::read_u32(slot, actor, err) || actor == 0 ||
            !guest_ok(actor, kActorVptr + 4, err))
            continue;
        if (filter) {
            uint32_t id = 0;
            if (!actor_profile_id(actor, id, err)) continue;
            if (id != want) continue;
        }
        ++count;
    }
    lua_pushnumber(L, count);
    return 1;
}

int l_level_spawn(lua_State* L) {
    if (!t_guest_cpu) return fail(L, "no guest cpu");
    if (!lua_isnumber(L, 1)) return fail(L, "bad type");
    double id = lua_tonumber(L, 1);
    if (id < 0 || id >= 913) return fail(L, "bad profile id");
    float x = 0, y = 0, z = 0;
    std::string err;
    if (!read_xyz(L, 2, x, y, z, err)) return fail(L, err.c_str());
    uint32_t param0 = 0, param1 = 0, layer = 0;
    if (lua_istable(L, 5)) {
        lua_getfield(L, 5, "param0");
        if (lua_isnumber(L, -1)) param0 = (uint32_t)lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, 5, "param1");
        if (lua_isnumber(L, -1)) param1 = (uint32_t)lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, 5, "layer");
        if (lua_isnumber(L, -1)) layer = (uint32_t)lua_tonumber(L, -1) & 0xFF;
        lua_pop(L, 1);
    }
    uint32_t mgr = 0;
    if (!actor_mgr(mgr, err)) return fail(L, err.c_str());
    uint32_t profile = 0;
    if (!luau_guest::read_u32(kProfileList + (uint32_t)id * 4, profile, err))
        return fail(L, err.c_str());
    if (profile == 0 || !guest_ok(profile, 0x14, err)) return fail(L, "unknown profile");
    uint32_t registered = 0;
    if (!luau_guest::read_u32(profile + kActorId, registered, err)) return fail(L, err.c_str());
    if (registered != (uint32_t)id) return fail(L, "unknown profile");
    static const uint32_t s_param = mem::host_alloc(0x2C, 0x20);
    if (s_param == 0) return fail(L, "out of memory");
    st32(s_param + 0x00, param0);
    st32(s_param + 0x04, param1);
    st32(s_param + 0x08, 0);
    st32(s_param + 0x0C, profile);
    stf32(s_param + 0x10, x);
    stf32(s_param + 0x14, y);
    stf32(s_param + 0x18, z);
    st32(s_param + 0x1C, 0);
    st8(s_param + 0x20, (uint8_t)layer);
    st8(s_param + 0x21, 0);
    st8(s_param + 0x22, 0);
    st8(s_param + 0x24, 0);
    st8(s_param + 0x25, 0);
    st8(s_param + 0x26, 0);
    st32(s_param + 0x28, 0);
    guest_call(t_guest_cpu, kActorMgrCreateLater, {mgr, s_param});
    lua_pushboolean(L, 1);
    return 1;
}

int l_actor_address(lua_State* L) {
    uint32_t actor = 0;
    std::string err;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)actor);
    return 1;
}

int l_actor_id(lua_State* L) {
    uint32_t actor = 0;
    std::string err;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    uint32_t id = 0;
    if (!luau_guest::read_u32(actor + kActorId, id, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)id);
    return 1;
}

int l_actor_profile_id(lua_State* L) {
    uint32_t actor = 0;
    std::string err;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    uint32_t id = 0;
    if (!actor_profile_id(actor, id, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)id);
    return 1;
}

int l_actor_is_actor(lua_State* L) {
    uint32_t actor = 0;
    std::string err;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    bool out = false;
    if (!actor_is_actor(actor, out, err)) return fail(L, err.c_str());
    lua_pushboolean(L, out ? 1 : 0);
    return 1;
}

int l_actor_is_map(lua_State* L) {
    uint32_t actor = 0;
    std::string err;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    uint8_t map = 0;
    if (!luau_guest::read_u8(actor + kActorIsMap, map, err)) return fail(L, err.c_str());
    lua_pushboolean(L, map ? 1 : 0);
    return 1;
}

int l_actor_active(lua_State* L) {
    uint32_t actor = 0;
    std::string err;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    uint8_t active = 0;
    if (!luau_guest::read_u8(actor + kActorIsActive, active, err)) return fail(L, err.c_str());
    lua_pushboolean(L, active ? 1 : 0);
    return 1;
}

int l_actor_layer(lua_State* L) {
    uint32_t actor = 0;
    std::string err;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    uint8_t layer = 0;
    if (!luau_guest::read_u8(actor + kLayer, layer, err)) return fail(L, err.c_str());
    lua_pushnumber(L, layer);
    return 1;
}

int l_actor_player_no(lua_State* L) {
    uint32_t actor = 0;
    std::string err;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    int8_t no = 0;
    if (!luau_guest::read_s8(actor + kPlayerNo, no, err)) return fail(L, err.c_str());
    lua_pushnumber(L, no);
    return 1;
}

int l_actor_param0(lua_State* L) {
    uint32_t actor = 0;
    std::string err;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    uint32_t param = 0;
    if (!luau_guest::read_u32(actor + kActorParam0, param, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)param);
    return 1;
}

int l_actor_param1(lua_State* L) {
    uint32_t actor = 0;
    std::string err;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    uint32_t param = 0;
    if (!luau_guest::read_u32(actor + kActorParam1, param, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)param);
    return 1;
}

int l_actor_pos(lua_State* L) {
    uint32_t actor = 0;
    std::string err;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    bool is_actor_value = false;
    if (!actor_is_actor(actor, is_actor_value, err)) return fail(L, err.c_str());
    if (!is_actor_value) return fail(L, "not an Actor");
    float x = 0, y = 0, z = 0;
    if (!read_vec3(actor, kPosX, kPosY, kPosZ, x, y, z, err)) return fail(L, err.c_str());
    push_vec3(L, x, y, z);
    return 1;
}

int l_actor_set_pos(lua_State* L) {
    float x = 0, y = 0, z = 0;
    std::string err;
    if (!read_xyz(L, 2, x, y, z, err)) return fail(L, err.c_str());
    uint32_t actor = 0;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    bool is_actor_value = false;
    if (!actor_is_actor(actor, is_actor_value, err)) return fail(L, err.c_str());
    if (!is_actor_value) return fail(L, "not an Actor");
    if (!write_vec3(actor, kPosX, kPosY, kPosZ, x, y, z, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_actor_speed(lua_State* L) {
    uint32_t actor = 0;
    std::string err;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    bool is_actor_value = false;
    if (!actor_is_actor(actor, is_actor_value, err)) return fail(L, err.c_str());
    if (!is_actor_value) return fail(L, "not an Actor");
    float x = 0, y = 0, z = 0;
    if (!read_vec3(actor, kSpeedX, kSpeedY, kSpeedZ, x, y, z, err)) return fail(L, err.c_str());
    push_vec3(L, x, y, z);
    return 1;
}

int l_actor_set_speed(lua_State* L) {
    float x = 0, y = 0, z = 0;
    std::string err;
    if (!read_xyz(L, 2, x, y, z, err)) return fail(L, err.c_str());
    uint32_t actor = 0;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    bool is_actor_value = false;
    if (!actor_is_actor(actor, is_actor_value, err)) return fail(L, err.c_str());
    if (!is_actor_value) return fail(L, "not an Actor");
    if (!write_vec3(actor, kSpeedX, kSpeedY, kSpeedZ, x, y, z, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_actor_scale(lua_State* L) {
    uint32_t actor = 0;
    std::string err;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    bool is_actor_value = false;
    if (!actor_is_actor(actor, is_actor_value, err)) return fail(L, err.c_str());
    if (!is_actor_value) return fail(L, "not an Actor");
    float x = 0, y = 0, z = 0;
    if (!read_vec3(actor, kScaleX, kScaleY, kScaleZ, x, y, z, err)) return fail(L, err.c_str());
    push_vec3(L, x, y, z);
    return 1;
}

int l_actor_set_scale(lua_State* L) {
    float x = 0, y = 0, z = 0;
    std::string err;
    if (!read_xyz(L, 2, x, y, z, err)) return fail(L, err.c_str());
    uint32_t actor = 0;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    bool is_actor_value = false;
    if (!actor_is_actor(actor, is_actor_value, err)) return fail(L, err.c_str());
    if (!is_actor_value) return fail(L, "not an Actor");
    if (!write_vec3(actor, kScaleX, kScaleY, kScaleZ, x, y, z, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_actor_delete(lua_State* L) {
    uint32_t actor = 0;
    std::string err;
    if (!handle_addr(L, actor, err)) return fail(L, err.c_str());
    if (!luau_guest::write_u8(actor + kActorDeleteFlag, 1, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_player_call(lua_State* L) {
    lua_remove(L, 1);
    return l_player(L);
}

void install_player_meta(lua_State* L) {
    if (luaL_newmetatable(L, kPlayerMeta)) {
        lua_newtable(L);
        bind_guest(L, "entered", l_player_entered);
        bind_guest(L, "lives", l_player_lives);
        bind_guest(L, "set_lives", l_player_set_lives);
        bind_guest(L, "mode", l_player_mode);
        bind_guest(L, "set_mode", l_player_set_mode);
        bind_guest(L, "coins", l_player_coins);
        bind_guest(L, "set_coins", l_player_set_coins);
        bind_guest(L, "speed_y", l_player_speed_y);
        bind_guest(L, "set_speed_y", l_player_set_speed_y);
        bind_guest(L, "pos", l_player_pos);
        bind_guest(L, "set_pos", l_player_set_pos);
        bind_guest(L, "speed", l_player_speed);
        bind_guest(L, "set_speed", l_player_set_speed);
        bind_guest(L, "grounded", l_player_grounded);
        bind_guest(L, "actor", l_player_actor);
        lua_setfield(L, -2, "__index");
    }
    lua_pop(L, 1);
}

void install_actor_meta(lua_State* L) {
    if (luaL_newmetatable(L, kActorMeta)) {
        lua_newtable(L);
        bind_guest(L, "address", l_actor_address);
        bind_guest(L, "id", l_actor_id);
        bind_guest(L, "profile_id", l_actor_profile_id);
        bind_guest(L, "is_actor", l_actor_is_actor);
        bind_guest(L, "is_map_actor", l_actor_is_map);
        bind_guest(L, "active", l_actor_active);
        bind_guest(L, "layer", l_actor_layer);
        bind_guest(L, "player_no", l_actor_player_no);
        bind_guest(L, "param0", l_actor_param0);
        bind_guest(L, "param1", l_actor_param1);
        bind_guest(L, "pos", l_actor_pos);
        bind_guest(L, "set_pos", l_actor_set_pos);
        bind_guest(L, "speed", l_actor_speed);
        bind_guest(L, "set_speed", l_actor_set_speed);
        bind_guest(L, "scale", l_actor_scale);
        bind_guest(L, "set_scale", l_actor_set_scale);
        bind_guest(L, "delete", l_actor_delete);
        lua_setfield(L, -2, "__index");
    }
    lua_pop(L, 1);
}

int l_scene_installed(lua_State* L, uint32_t where) {
    uint32_t inst;
    std::string err;
    if (!luau_guest::read_u32(where, inst, err)) return fail(L, err.c_str());
    lua_pushboolean(L, inst != 0);
    return 1;
}

bool scene_object(uint32_t& out, std::string& err) {
    uint32_t scene = 0;
    if (!luau_guest::read_u32(kSceneSingleton, scene, err)) return false;
    if (scene == 0) {
        err = "scene not ready";
        return false;
    }
    out = scene;
    return true;
}

bool scene_now_creator(uint32_t& out, std::string& err) {
    uint32_t scene = 0;
    if (!scene_object(scene, err)) return false;
    uint32_t creator = 0;
    if (!luau_guest::read_u32(scene + kSceneNowCreatorOff, creator, err)) return false;
    if (creator == 0) {
        err = "scene not ready";
        return false;
    }
    out = creator;
    return true;
}

bool scene_state_is(lua_State* L, uint32_t creator) {
    uint32_t now = 0;
    std::string err;
    if (!scene_now_creator(now, err)) return fail(L, err.c_str());
    lua_pushboolean(L, now == creator);
    return 1;
}

int l_scene_title(lua_State* L) { return scene_state_is(L, kCreatorTitle); }

int l_scene_file_select(lua_State* L) { return scene_state_is(L, kCreatorFileSelect); }

int l_scene_flow(lua_State* L) { return scene_state_is(L, kCreatorFlow); }

int l_scene_map_phase(lua_State* L) { return scene_state_is(L, kCreatorMap); }

int l_scene_phase(lua_State* L) {
    uint32_t now = 0;
    std::string err;
    if (!scene_now_creator(now, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)now);
    return 1;
}

int l_scene_course_select(lua_State* L) { return l_scene_installed(L, kCourseSelectInstance); }

int l_scene_course(lua_State* L) { return l_scene_installed(L, kCourseTaskInstance); }

bool scene_app_object(uint32_t& out, std::string& err) {
    uint32_t scene = 0;
    if (!scene_object(scene, err)) return false;
    uint32_t ref = 0;
    if (!luau_guest::read_u32(scene + kSceneFlowOff, ref, err)) return false;
    if (ref == 0) {
        err = "scene not ready";
        return false;
    }
    out = ref - 4;
    return true;
}

bool scene_transition(uint32_t creator, uint32_t fader, std::string& err) {
    if (!t_guest_cpu) {
        err = "no guest cpu";
        return false;
    }
    uint32_t app = 0;
    if (!scene_app_object(app, err)) return false;
    uint32_t pair = mem::host_alloc(8, 4);
    st32(pair, kClassIdTask);
    st32(pair + 4, creator);
    guest_call(t_guest_cpu, kSetNextScene, {app, 0, pair, fader, 0});
    return true;
}

int l_scene_open_title(lua_State* L) {
    std::string err;
    if (!scene_transition(kCreatorTitle, 0, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_scene_open_file_select(lua_State* L) {
    if (!t_guest_cpu) return fail(L, "no guest cpu");
    uint32_t app = 0;
    std::string err;
    if (!scene_app_object(app, err)) return fail(L, err.c_str());
    guest_call(t_guest_cpu, kAppOpenFileSelect, {app});
    lua_pushboolean(L, 1);
    return 1;
}

int l_scene_open_course_select(lua_State* L) {
    std::string err;
    if (!scene_transition(kCreatorCourseSelect, 0x000A000Au, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_scene_confirm_file(lua_State* L) {
    if (!t_guest_cpu) return fail(L, "no guest cpu");
    uint32_t app = 0;
    std::string err;
    if (!scene_app_object(app, err)) return fail(L, err.c_str());
    if (!luau_guest::write_u8(app + kAppGateSelect, 1, err)) return fail(L, err.c_str());
    if (!luau_guest::write_u8(app + kAppGateStory, 1, err)) return fail(L, err.c_str());
    guest_call(t_guest_cpu, kAppConfirmFile, {app});
    lua_pushboolean(L, 1);
    return 1;
}

int l_scene_load_story(lua_State* L) {
    if (!t_guest_cpu) return fail(L, "no guest cpu");
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) return fail(L, "bad type");
    int32_t world = (int32_t)lua_tonumber(L, 1);
    int32_t course = (int32_t)lua_tonumber(L, 2);
    if (world < 0 || world > 8) return fail(L, "bad world");
    if (course < 0 || course > 11) return fail(L, "bad course");
    uint32_t scene = 0;
    std::string err;
    if (!scene_object(scene, err)) return fail(L, err.c_str());
    guest_call(t_guest_cpu, kSetCourseStory, {scene, 0, (uint32_t)world, (uint32_t)course});
    lua_pushboolean(L, 1);
    return 1;
}

void install_game(lua_State* L) {
    install_player_meta(L);
    install_actor_meta(L);

    lua_newtable(L);
    bind_guest(L, "active", l_course_active);
    bind_guest(L, "time", l_course_time);
    bind_guest(L, "set_time", l_course_set_time);
    bind_guest(L, "add_time", l_course_add_time);
    bind_guest(L, "coins", l_course_coins);
    bind_guest(L, "set_coins", l_course_set_coins);
    bind_guest(L, "score", l_course_score);
    bind_guest(L, "set_score", l_course_set_score);
    bind_guest(L, "actors", l_level_actors);
    bind_guest(L, "count", l_level_count);
    bind_guest(L, "spawn", l_level_spawn);
    set_loaded(L, "nsmbu.level");

    lua_newtable(L);
    bind_guest(L, "get_player", l_player);
    bind_guest(L, "get_players", l_players);
    set_loaded(L, "nsmbu.player");

    lua_newtable(L);
    bind_guest(L, "title", l_scene_title);
    bind_guest(L, "file_select", l_scene_file_select);
    bind_guest(L, "flow_phase", l_scene_flow);
    bind_guest(L, "map_phase", l_scene_map_phase);
    bind_guest(L, "phase", l_scene_phase);
    bind_guest(L, "course_select", l_scene_course_select);
    bind_guest(L, "course", l_scene_course);
    bind_guest(L, "open_title", l_scene_open_title);
    bind_guest(L, "open_file_select", l_scene_open_file_select);
    bind_guest(L, "open_course_select", l_scene_open_course_select);
    bind_guest(L, "confirm_file", l_scene_confirm_file);
    bind_guest(L, "load_story", l_scene_load_story);
    set_loaded(L, "nsmbu.scene");
}

void install_global(lua_State* L) {
    if (!push_loaded(L, "nsmbu")) {
        lua_pop(L, 1);
        return;
    }
    attach_loaded(L, "nsmbu.config", "config");
    attach_loaded(L, "nsmbu.guest", "guest");
    attach_loaded(L, "nsmbu.fn", "fn");
    attach_loaded(L, "nsmbu.hooks", "hooks");
    attach_loaded(L, "nsmbu.input", "input");
    attach_loaded(L, "nsmbu.data", "data");
    attach_loaded(L, "nsmbu.enemy", "enemy");
    attach_loaded(L, "nsmbu.boss", "boss");
    attach_loaded(L, "nsmbu.blocks", "blocks");
    attach_loaded(L, "nsmbu.env", "env");
    attach_loaded(L, "nsmbu.level", "level");
    attach_loaded(L, "nsmbu.player", "player");
    attach_loaded(L, "nsmbu.scene", "scene");
    lua_setglobal(L, "nsmbu");
}

void install_modules(lua_State* L, Instance* inst) {
    lua_newtable(L);

    bind(L, inst, "on_logic_step", l_on_logic_step);
    bind(L, inst, "on_config_changed", l_on_config_changed);
    bind(L, inst, "on_unload", l_on_unload);
    bind(L, inst, "unlisten", l_unlisten);
    bind(L, inst, "log", l_log);
    bind(L, inst, "log_int", l_log_int);
    bind(L, inst, "log_hex", l_log_hex);
    bind(L, inst, "log_float", l_log_float);
    bind(L, inst, "status", l_status);
    bind(L, inst, "logic_step", l_logic_step);
    bind(L, inst, "logic_dt", l_logic_dt);

    lua_newtable(L);
    lua_pushlstring(L, inst->id.data(), inst->id.size());
    lua_setfield(L, -2, "id");
    lua_pushlstring(L, inst->version.data(), inst->version.size());
    lua_setfield(L, -2, "version");
    lua_pushlstring(L, inst->path.data(), inst->path.size());
    lua_setfield(L, -2, "path");
    lua_setfield(L, -2, "package");

    set_loaded(L, "nsmbu");

    lua_newtable(L);
    bind(L, inst, "string", l_config_string);
    bind(L, inst, "number", l_config_number);
    bind(L, inst, "bool", l_config_bool);
    set_loaded(L, "nsmbu.config");

    lua_newtable(L);
    bind_guest(L, "read_u8", guest_read_u8);
    bind_guest(L, "read_s8", guest_read_s8);
    bind_guest(L, "read_u16", guest_read_u16);
    bind_guest(L, "read_s16", guest_read_s16);
    bind_guest(L, "read_u32", guest_read_u32);
    bind_guest(L, "read_s32", guest_read_s32);
    bind_guest(L, "read_f32", guest_read_f32);
    bind_guest(L, "read_f64", guest_read_f64);
    bind_guest(L, "read", guest_read);
    bind_guest(L, "write_u8", guest_write_u8);
    bind_guest(L, "write_s8", guest_write_s8);
    bind_guest(L, "write_u16", guest_write_u16);
    bind_guest(L, "write_s16", guest_write_s16);
    bind_guest(L, "write_u32", guest_write_u32);
    bind_guest(L, "write_s32", guest_write_s32);
    bind_guest(L, "write_f32", guest_write_f32);
    bind_guest(L, "write_f64", guest_write_f64);
    bind_guest(L, "write", guest_write);
    set_loaded(L, "nsmbu.guest");

    set_empty_module(L, "nsmbu.fn");
    set_empty_module(L, "nsmbu.hooks");
    lua_newtable(L);
    bind(L, inst, "press", l_input_press);
    set_loaded(L, "nsmbu.input");
    set_empty_module(L, "nsmbu.data");
    set_empty_module(L, "nsmbu.enemy");
    set_empty_module(L, "nsmbu.boss");
    set_empty_module(L, "nsmbu.blocks");
    set_empty_module(L, "nsmbu.env");

    install_game(L);
}
}

void install(lua_State* L, Instance* inst) {
    install_modules(L, inst);
    install_global(L);
}

void set_cpu(Cpu* c) {
    t_guest_cpu = c;
}

Cpu* cpu() {
    return t_guest_cpu;
}

}
