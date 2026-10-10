#include "lua_game.h"
#include "lua_guest.h"
#include <cstdint>
#include <string>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

namespace mods::lua_game {
namespace {

constexpr uint32_t kCourseTimerSingleton = 0x101D15F4u;
constexpr uint32_t kFieldGameSingleton = 0x101D1604u;
constexpr uint32_t kPlayerMgrSingleton = 0x101E6994u;

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
constexpr uint32_t kPosX = 0x6Cu;
constexpr uint32_t kPosY = 0x70u;
constexpr uint32_t kPosZ = 0x74u;
constexpr uint32_t kSpeedX = 0x78u;
constexpr uint32_t kSpeedY = 0x7Cu;
constexpr uint32_t kSpeedZ = 0x80u;
constexpr uint32_t kActorMode = 0x500u;
constexpr uint32_t kNowBgCross = 0x1AC8u;
constexpr uint32_t kBgCrossIsFoot = 1u;

constexpr int kPlayerCount = 4;
constexpr const char* kPlayerMeta = "nsmbu.player";

int fail(lua_State* L, const char* msg) {
    lua_pushnil(L);
    lua_pushstring(L, msg);
    return 2;
}

bool guest_ok(uint32_t addr, size_t size, std::string& err) {
    return lua_guest::check(addr, size, err);
}

bool read_ptr(uint32_t addr, uint32_t& out, std::string& err) {
    uint32_t p = 0;
    if (!lua_guest::read_u32(addr, p, err)) return false;
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
    if (!lua_guest::read_u32(mgr + kPlayerObjects + (uint32_t)index * 4u, actor, err)) return false;
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
    if (!lua_guest::read_u8(slot + kEntryOff, entry, err)) return false;
    out = entry != 0;
    return true;
}

int player_index(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "index");
    if (!lua_isnumber(L, -1)) return luaL_error(L, "player index missing");
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
    if (!lua_guest::read_f32(base + ox, x, err)) return false;
    if (!lua_guest::read_f32(base + oy, y, err)) return false;
    if (!lua_guest::read_f32(base + oz, z, err)) return false;
    return true;
}

bool write_vec3(uint32_t base, uint32_t ox, uint32_t oy, uint32_t oz, float x, float y, float z,
                std::string& err) {
    if (!lua_guest::write_f32(base + ox, x, err)) return false;
    if (!lua_guest::write_f32(base + oy, y, err)) return false;
    if (!lua_guest::write_f32(base + oz, z, err)) return false;
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
    if (!lua_guest::read_u32(timer + kTimerTime, units, err)) return fail(L, err.c_str());
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
    if (!lua_guest::write_u32(timer + kTimerTime, units, err)) return fail(L, err.c_str());
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
    if (!lua_guest::read_u32(timer + kTimerTime, units, err)) return fail(L, err.c_str());
    double seconds = (double)(units >> 12) + delta;
    if (seconds < 0) seconds = 0;
    if (seconds > 999) seconds = 999;
    units = ((uint32_t)seconds) << 12;
    if (!lua_guest::write_u32(timer + kTimerTime, units, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_course_coins(lua_State* L) {
    uint32_t field = 0;
    std::string err;
    if (!field_game(field, err)) return fail(L, err.c_str());
    int32_t coins = 0;
    if (!lua_guest::read_s32(field + kCourseCoinNum, coins, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)coins);
    return 1;
}

int l_course_set_coins(lua_State* L) {
    if (!lua_isnumber(L, 1)) return fail(L, "bad type");
    uint32_t field = 0;
    std::string err;
    if (!field_game(field, err)) return fail(L, err.c_str());
    int32_t coins = (int32_t)lua_tonumber(L, 1);
    if (!lua_guest::write_s32(field + kCourseCoinNum, coins, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_course_score(lua_State* L) {
    uint32_t field = 0;
    std::string err;
    if (!field_game(field, err)) return fail(L, err.c_str());
    uint32_t score = 0;
    if (!lua_guest::read_u32(field + kCurrentScore, score, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)score);
    return 1;
}

int l_course_set_score(lua_State* L) {
    if (!lua_isnumber(L, 1)) return fail(L, "bad type");
    uint32_t field = 0;
    std::string err;
    if (!field_game(field, err)) return fail(L, err.c_str());
    uint32_t score = (uint32_t)lua_tonumber(L, 1);
    if (!lua_guest::write_u32(field + kCurrentScore, score, err)) return fail(L, err.c_str());
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

int l_players_next(lua_State* L) {
    int i = (int)lua_tointeger(L, lua_upvalueindex(1));
    std::string err;
    while (i + 1 < kPlayerCount) {
        i += 1;
        lua_pushinteger(L, i);
        lua_replace(L, lua_upvalueindex(1));
        bool entered = false;
        if (!player_entered(i, entered, err)) continue;
        if (!entered) continue;
        push_player(L, i);
        return 1;
    }
    return 0;
}

int l_players(lua_State* L) {
    lua_pushinteger(L, -1);
    lua_pushcclosure(L, l_players_next, 1);
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
    if (!lua_guest::read_s32(slot + kLifeOff, lives, err)) return fail(L, err.c_str());
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
    if (!lua_guest::write_s32(slot + kLifeOff, lives, err)) return fail(L, err.c_str());
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
    if (!lua_guest::read_s32(slot + kModeOff, mode, err)) return fail(L, err.c_str());
    uint32_t actor = 0;
    if (actor_addr(index, actor, err)) {
        int32_t actor_mode = 0;
        if (lua_guest::read_s32(actor + kActorMode, actor_mode, err) && actor_mode >= 0 &&
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
    if (!lua_guest::write_s32(slot + kModeOff, mode, err)) return fail(L, err.c_str());
    uint32_t actor = 0;
    if (actor_addr(index, actor, err)) {
        if (!lua_guest::write_s32(actor + kActorMode, mode, err)) return fail(L, err.c_str());
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
    if (!lua_guest::read_s32(slot + kCoinOff, coins, err)) return fail(L, err.c_str());
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
    if (!lua_guest::write_s32(slot + kCoinOff, coins, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

int l_player_speed_y(lua_State* L) {
    int index = player_index(L);
    uint32_t actor = 0;
    std::string err;
    if (!actor_addr(index, actor, err)) return fail(L, err.c_str());
    float speed = 0;
    if (!lua_guest::read_f32(actor + kSpeedY, speed, err)) return fail(L, err.c_str());
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
    if (!lua_guest::write_f32(actor + kSpeedY, speed, err)) return fail(L, err.c_str());
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
    if (!lua_guest::read_u32(actor + kNowBgCross, cross, err)) return fail(L, err.c_str());
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

void bind(lua_State* L, const char* name, lua_CFunction fn) {
    lua_pushcfunction(L, fn);
    lua_setfield(L, -2, name);
}

void set_loaded(lua_State* L, const char* name) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "loaded");
    lua_pushvalue(L, -3);
    lua_setfield(L, -2, name);
    lua_pop(L, 3);
}

int l_player_call(lua_State* L) {
    lua_remove(L, 1);
    return l_player(L);
}

void install_player_meta(lua_State* L) {
    if (luaL_newmetatable(L, kPlayerMeta)) {
        lua_newtable(L);
        bind(L, "entered", l_player_entered);
        bind(L, "lives", l_player_lives);
        bind(L, "set_lives", l_player_set_lives);
        bind(L, "mode", l_player_mode);
        bind(L, "set_mode", l_player_set_mode);
        bind(L, "coins", l_player_coins);
        bind(L, "set_coins", l_player_set_coins);
        bind(L, "speed_y", l_player_speed_y);
        bind(L, "set_speed_y", l_player_set_speed_y);
        bind(L, "pos", l_player_pos);
        bind(L, "set_pos", l_player_set_pos);
        bind(L, "speed", l_player_speed);
        bind(L, "set_speed", l_player_set_speed);
        bind(L, "grounded", l_player_grounded);
        bind(L, "actor", l_player_actor);
        lua_setfield(L, -2, "__index");
    }
    lua_pop(L, 1);
}

}

void install(lua_State* L) {
    install_player_meta(L);

    lua_newtable(L);
    bind(L, "active", l_course_active);
    bind(L, "time", l_course_time);
    bind(L, "set_time", l_course_set_time);
    bind(L, "add_time", l_course_add_time);
    bind(L, "coins", l_course_coins);
    bind(L, "set_coins", l_course_set_coins);
    bind(L, "score", l_course_score);
    bind(L, "set_score", l_course_set_score);
    set_loaded(L, "nsmbu.level");

    lua_newtable(L);
    bind(L, "get", l_player);
    bind(L, "players", l_players);
    lua_newtable(L);
    lua_pushcfunction(L, l_player_call);
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);
    set_loaded(L, "nsmbu.player");
}

}
