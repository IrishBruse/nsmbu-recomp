#include "lua_runtime.h"
#include "lua_guest.h"
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stdexcept>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace mods::lua_mod {
namespace {
namespace fs = std::filesystem;
using json::Value;

Instance* self(lua_State* L) {
    return static_cast<Instance*>(lua_touserdata(L, lua_upvalueindex(1)));
}

int fail(lua_State* L, const char* msg) {
    lua_pushnil(L);
    lua_pushstring(L, msg);
    return 2;
}

void set_fault(Instance& inst, const char* msg) {
    inst.fault = true;
    inst.fault_msg = msg ? msg : "lua error";
    fprintf(stderr, "[mod:%s] %s\n", inst.id.c_str(), inst.fault_msg.c_str());
}

void nil_global(lua_State* L, const char* name) {
    lua_pushnil(L);
    lua_setglobal(L, name);
}

void open_lib(lua_State* L, lua_CFunction fn, const char* name) {
    lua_pushcfunction(L, fn);
    lua_pushstring(L, name);
    lua_call(L, 1, 0);
}

bool relative_ok(const std::string& name) {
    if (name.empty() || name.size() > 512 || name.find('\\') != std::string::npos ||
        name.find(':') != std::string::npos || name.find('\0') != std::string::npos)
        return false;
    fs::path p(name);
    if (p.is_absolute()) return false;
    for (const auto& part : p)
        if (part == ".." || part == "." || part.empty()) return false;
    return true;
}

bool resolve_under(const fs::path& root, std::string rel, fs::path& out, std::string& err) {
    if (!relative_ok(rel)) {
        err = "path not allowed";
        return false;
    }
    auto root_n = root.lexically_normal();
    out = (root_n / rel).lexically_normal();
    auto check = out.lexically_relative(root_n);
    if (check.empty()) {
        err = "path not allowed";
        return false;
    }
    for (const auto& part : check)
        if (part == "..") {
            err = "path not allowed";
            return false;
        }
    return true;
}

std::string module_to_rel(std::string name) {
    for (char& c : name)
        if (c == '.') c = '/';
    if (name.size() < 4 || name.compare(name.size() - 4, 4, ".lua") != 0) name += ".lua";
    return name;
}

void refresh_package(Instance& inst) {
    lua_State* L = static_cast<lua_State*>(inst.state);
    if (!L) return;
    lua_getglobal(L, "nsmbu");
    if (!lua_istable(L, -1)) {
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

bool call_fn(Instance& inst, int nargs) {
    lua_State* L = static_cast<lua_State*>(inst.state);
    if (lua_pcall(L, nargs, 0, 0) != 0) {
        set_fault(inst, lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    return true;
}

bool call_field(Instance& inst, const char* field, int nargs_after_fn) {
    if (inst.fault || !inst.state) return false;
    refresh_package(inst);
    lua_State* L = static_cast<lua_State*>(inst.state);
    int top = lua_gettop(L) - nargs_after_fn;
    lua_getglobal(L, "nsmbu");
    if (!lua_istable(L, -1)) {
        lua_settop(L, top);
        return true;
    }
    lua_getfield(L, -1, field);
    lua_remove(L, -2);
    if (lua_isnil(L, -1)) {
        lua_settop(L, top);
        return true;
    }
    if (!lua_isfunction(L, -1)) {
        lua_settop(L, top);
        return true;
    }
    if (nargs_after_fn > 0) lua_insert(L, -1 - nargs_after_fn);
    return call_fn(inst, nargs_after_fn);
}

bool fire_logic(Instance& inst) {
    lua_State* L = static_cast<lua_State*>(inst.state);
    lua_pushnumber(L, (lua_Number)inst.step);
    if (!call_field(inst, "on_logic_step", 1)) return false;
    lua_pushnumber(L, (lua_Number)inst.step);
    std::vector<int> refs;
    for (const auto& l : inst.listeners)
        if (l.event == "logic_step") refs.push_back(l.ref);
    for (int ref : refs) {
        if (inst.fault) return false;
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        lua_pushvalue(L, -2);
        if (!call_fn(inst, 1)) {
            lua_pop(L, 1);
            return false;
        }
    }
    lua_pop(L, 1);
    return true;
}

bool fire_config(Instance& inst) {
    if (!call_field(inst, "on_config_changed", 0)) return false;
    lua_State* L = static_cast<lua_State*>(inst.state);
    for (const auto& l : inst.listeners) {
        if (l.event != "config_changed") continue;
        if (inst.fault) return false;
        lua_rawgeti(L, LUA_REGISTRYINDEX, l.ref);
        if (!call_fn(inst, 0)) return false;
    }
    return true;
}

bool fire_unload(Instance& inst) {
    if (!call_field(inst, "on_unload", 0)) return false;
    lua_State* L = static_cast<lua_State*>(inst.state);
    for (size_t i = inst.listeners.size(); i > 0; --i) {
        const auto& l = inst.listeners[i - 1];
        if (l.event != "unload") continue;
        if (inst.fault) return false;
        lua_rawgeti(L, LUA_REGISTRYINDEX, l.ref);
        if (!call_fn(inst, 0)) return false;
    }
    return true;
}

int l_require(lua_State* L) {
    Instance* inst = self(L);
    const char* modname = luaL_checkstring(L, 1);
    lua_getglobal(L, "package");
    if (!lua_istable(L, -1)) return luaL_error(L, "package table missing");
    lua_getfield(L, -1, "loaded");
    lua_remove(L, -2);
    if (!lua_istable(L, -1)) return luaL_error(L, "package.loaded missing");
    lua_getfield(L, -1, modname);
    if (!lua_isnil(L, -1)) {
        lua_remove(L, -2);
        return 1;
    }
    lua_pop(L, 1);

    std::string err;
    fs::path file;
    if (!resolve_under(fs::path(inst->path), module_to_rel(modname), file, err))
        return luaL_error(L, "%s", err.c_str());
    if (!fs::is_regular_file(file)) return luaL_error(L, "module not found: %s", modname);
    if (luaL_loadfile(L, file.string().c_str()) != 0) return lua_error(L);
    lua_pushvalue(L, 1);
    if (lua_pcall(L, 1, 1, 0) != 0) return lua_error(L);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_pushboolean(L, 1);
    }
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, modname);
    lua_remove(L, -2);
    return 1;
}

int l_listen(lua_State* L) {
    Instance* inst = self(L);
    const char* event = luaL_checkstring(L, 1);
    if (std::strcmp(event, "logic_step") && std::strcmp(event, "config_changed") &&
        std::strcmp(event, "unload"))
        return fail(L, "unknown event");
    if (!lua_isfunction(L, 2)) return fail(L, "callback must be a function");
    lua_pushvalue(L, 2);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    int id = inst->next_listener++;
    inst->listeners.push_back({id, ref, event});
    lua_pushinteger(L, id);
    return 1;
}

int l_unlisten(lua_State* L) {
    Instance* inst = self(L);
    int id = (int)luaL_checkinteger(L, 1);
    for (auto it = inst->listeners.begin(); it != inst->listeners.end(); ++it) {
        if (it->id != id) continue;
        luaL_unref(L, LUA_REGISTRYINDEX, it->ref);
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
    if (!lua_guest::read_u8(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushinteger(L, v);
    return 1;
}
int guest_read_s8(lua_State* L) {
    std::string err;
    int8_t v;
    if (!lua_guest::read_s8(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushinteger(L, v);
    return 1;
}
int guest_read_u16(lua_State* L) {
    std::string err;
    uint16_t v;
    if (!lua_guest::read_u16(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushinteger(L, v);
    return 1;
}
int guest_read_s16(lua_State* L) {
    std::string err;
    int16_t v;
    if (!lua_guest::read_s16(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushinteger(L, v);
    return 1;
}
int guest_read_u32(lua_State* L) {
    std::string err;
    uint32_t v;
    if (!lua_guest::read_u32(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)v);
    return 1;
}
int guest_read_s32(lua_State* L) {
    std::string err;
    int32_t v;
    if (!lua_guest::read_s32(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushnumber(L, (lua_Number)v);
    return 1;
}
int guest_read_f32(lua_State* L) {
    std::string err;
    float v;
    if (!lua_guest::read_f32(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushnumber(L, v);
    return 1;
}
int guest_read_f64(lua_State* L) {
    std::string err;
    double v;
    if (!lua_guest::read_f64(addr_arg(L, 1), v, err)) return fail(L, err.c_str());
    lua_pushnumber(L, v);
    return 1;
}
int guest_read(lua_State* L) {
    uint32_t addr = addr_arg(L, 1);
    size_t size = (size_t)luaL_checkinteger(L, 2);
    std::string err, out;
    if (!lua_guest::read_bytes(addr, size, out, err)) return fail(L, err.c_str());
    lua_pushlstring(L, out.data(), out.size());
    return 1;
}

int guest_write_u8(lua_State* L) {
    std::string err;
    if (!lua_guest::write_u8(addr_arg(L, 1), (uint8_t)(uint32_t)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write_s8(lua_State* L) {
    std::string err;
    if (!lua_guest::write_s8(addr_arg(L, 1), (int8_t)(int32_t)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write_u16(lua_State* L) {
    std::string err;
    if (!lua_guest::write_u16(addr_arg(L, 1), (uint16_t)(uint32_t)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write_s16(lua_State* L) {
    std::string err;
    if (!lua_guest::write_s16(addr_arg(L, 1), (int16_t)(int32_t)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write_u32(lua_State* L) {
    std::string err;
    if (!lua_guest::write_u32(addr_arg(L, 1), (uint32_t)(int64_t)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write_s32(lua_State* L) {
    std::string err;
    if (!lua_guest::write_s32(addr_arg(L, 1), (int32_t)(int64_t)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write_f32(lua_State* L) {
    std::string err;
    if (!lua_guest::write_f32(addr_arg(L, 1), (float)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write_f64(lua_State* L) {
    std::string err;
    if (!lua_guest::write_f64(addr_arg(L, 1), (double)luaL_checknumber(L, 2), err))
        return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}
int guest_write(lua_State* L) {
    uint32_t addr = addr_arg(L, 1);
    size_t len = 0;
    const char* data = luaL_checklstring(L, 2, &len);
    std::string err;
    if (!lua_guest::write_bytes(addr, data, len, err)) return fail(L, err.c_str());
    lua_pushboolean(L, 1);
    return 1;
}

void bind(lua_State* L, Instance* inst, const char* name, lua_CFunction fn) {
    lua_pushlightuserdata(L, inst);
    lua_pushcclosure(L, fn, 1);
    lua_setfield(L, -2, name);
}

void bind_guest(lua_State* L, const char* name, lua_CFunction fn) {
    lua_pushcfunction(L, fn);
    lua_setfield(L, -2, name);
}

void install_nsmbu(lua_State* L, Instance* inst) {
    lua_newtable(L);

    bind(L, inst, "listen", l_listen);
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

    lua_newtable(L);
    bind(L, inst, "string", l_config_string);
    bind(L, inst, "number", l_config_number);
    bind(L, inst, "bool", l_config_bool);
    lua_setfield(L, -2, "config");

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
    lua_setfield(L, -2, "guest");

    lua_newtable(L);
    lua_setfield(L, -2, "fn");

    lua_setglobal(L, "nsmbu");
}

void sandbox(lua_State* L, Instance* inst) {
    open_lib(L, luaopen_base, "");
    open_lib(L, luaopen_string, LUA_STRLIBNAME);
    open_lib(L, luaopen_table, LUA_TABLIBNAME);
    open_lib(L, luaopen_math, LUA_MATHLIBNAME);
    open_lib(L, luaopen_bit, LUA_BITLIBNAME);

    nil_global(L, "dofile");
    nil_global(L, "loadfile");
    nil_global(L, "io");
    nil_global(L, "os");
    nil_global(L, "debug");
    nil_global(L, "jit");
    nil_global(L, "ffi");

    lua_newtable(L);
    lua_newtable(L);
    lua_setfield(L, -2, "loaded");
    lua_newtable(L);
    lua_setfield(L, -2, "preload");
    lua_pushstring(L, "");
    lua_setfield(L, -2, "path");
    lua_pushstring(L, "");
    lua_setfield(L, -2, "cpath");
    lua_pushnil(L);
    lua_setfield(L, -2, "loadlib");
    lua_setglobal(L, "package");

    lua_pushlightuserdata(L, inst);
    lua_pushcclosure(L, l_require, 1);
    lua_setglobal(L, "require");
}

void close_state(Instance& inst) {
    if (!inst.state) return;
    lua_State* L = static_cast<lua_State*>(inst.state);
    for (auto& l : inst.listeners) luaL_unref(L, LUA_REGISTRYINDEX, l.ref);
    inst.listeners.clear();
    lua_close(L);
    inst.state = nullptr;
}
}

Instance::Instance() = default;
Instance::~Instance() { close_state(*this); }
Instance::Instance(Instance&& o) noexcept
    : id(std::move(o.id)), version(std::move(o.version)), path(std::move(o.path)),
      entry(std::move(o.entry)), config(std::move(o.config)), status_line(std::move(o.status_line)),
      fault_msg(std::move(o.fault_msg)), fault(o.fault), step(o.step), state(o.state),
      listeners(std::move(o.listeners)), next_listener(o.next_listener) {
    o.state = nullptr;
}
Instance& Instance::operator=(Instance&& o) noexcept {
    if (this == &o) return *this;
    close_state(*this);
    id = std::move(o.id);
    version = std::move(o.version);
    path = std::move(o.path);
    entry = std::move(o.entry);
    config = std::move(o.config);
    status_line = std::move(o.status_line);
    fault_msg = std::move(o.fault_msg);
    fault = o.fault;
    step = o.step;
    state = o.state;
    listeners = std::move(o.listeners);
    next_listener = o.next_listener;
    o.state = nullptr;
    return *this;
}

std::unique_ptr<Instance> load(const LoadInfo& info) {
    if (info.id.empty()) throw std::runtime_error("mod id is empty");
    if (info.path.empty()) throw std::runtime_error("mod path is empty");
    if (info.entry.empty()) throw std::runtime_error("mod entry is empty");

    auto inst = std::make_unique<Instance>();
    inst->id = info.id;
    inst->version = info.version;
    inst->path = info.path;
    inst->entry = info.entry;
    inst->config = info.config;

    lua_State* L = luaL_newstate();
    if (!L) throw std::runtime_error("luaL_newstate failed");
    inst->state = L;

    sandbox(L, inst.get());
    install_nsmbu(L, inst.get());

    std::string err;
    fs::path entry_path;
    if (!resolve_under(fs::path(inst->path), info.entry, entry_path, err))
        throw std::runtime_error(err);
    if (!fs::is_regular_file(entry_path))
        throw std::runtime_error("entry script not found: " + info.entry);
    if (luaL_loadfile(L, entry_path.string().c_str()) != 0) {
        std::string msg = lua_tostring(L, -1) ? lua_tostring(L, -1) : "load failed";
        throw std::runtime_error(msg);
    }
    if (lua_pcall(L, 0, 0, 0) != 0) {
        std::string msg = lua_tostring(L, -1) ? lua_tostring(L, -1) : "entry failed";
        throw std::runtime_error(msg);
    }
    return inst;
}

void unload(Instance& inst) {
    if (!inst.state) return;
    fire_unload(inst);
    close_state(inst);
}

void set_config(Instance& inst, const json::Value& config) { inst.config = config; }

void config_changed(Instance& inst) {
    if (!inst.state || inst.fault) return;
    fire_config(inst);
}

void logic_step(Instance& inst, uint64_t step) {
    if (!inst.state || inst.fault) return;
    inst.step = step;
    fire_logic(inst);
}

std::string status(const Instance& inst) { return inst.status_line; }
bool faulted(const Instance& inst) { return inst.fault; }
std::string fault_message(const Instance& inst) { return inst.fault_msg; }

}
