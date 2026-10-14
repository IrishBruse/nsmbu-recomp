#include "mods/packages.h"
#include "mods/content.h"
#include "mods/luau_guest.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace {
void env(const char* key, const char* value) {
#ifdef _WIN32
    _putenv_s(key, value ? value : "");
#else
    if (value) setenv(key,value,1); else unsetenv(key);
#endif
}
}
#include "mods/cemu_pack.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
namespace {
mods::packages::View view(const std::string& id) {for(auto v:mods::packages::list())if(v.id==id)return v;return {};}
}
int main(int argc, char** argv) {
    namespace fs=std::filesystem;
    using namespace mods::packages;
    if(argc==2&&std::string(argv[1])=="--luau"){
        auto root=fs::temp_directory_path()/("nsmbu-luau-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        auto storage=root/"storage";fs::create_directories(storage);
        env("NSMBU_NO_HOST_INPUT","1");env("NSMBU_MOD_MANAGER_DIR",storage.string().c_str());
        initialize();
        std::string error;
        auto luau_pack=[&](const char* id,const char* main_src,const char* extra_manifest,const char* sib_src=nullptr){
            auto path=root/id;fs::create_directories(path);
            std::ofstream(path/"main.luau")<<main_src;
            if(sib_src)std::ofstream(path/"sib.luau")<<sib_src;
            std::ofstream(path/"manifest.json")<<"{\"format_version\":2,\"id\":\""<<id
              <<"\",\"name\":\""<<id<<"\",\"version\":\"1.0.0\",\"game_id\":\"nsmbu-usa\","
              <<"\"kind\":\"luau\",\"luau\":{\"api_version\":1,\"entry\":\"main.luau\"}"
              <<extra_manifest<<"}";
            return path.string();
        };
        assert(install(luau_pack("luau.status",
            "assert(type(rawget(_G,\"nsmbu\"))==\"table\")\n"
            "assert(nsmbu==require(\"nsmbu\"))\n"
            "assert(nsmbu.config==require(\"nsmbu.config\"))\n"
            "assert(type(nsmbu.level)==\"table\")\n"
            "assert(type(nsmbu.player)==\"table\")\n"
            "function nsmbu.hooks.on_logic_step(step)\n"
            "  nsmbu.status(nsmbu.config.string(\"label\",\"x\")..\":\"..tostring(step))\n"
            "end\n",
            ",\"options\":[{\"id\":\"label\",\"name\":\"Label\",\"type\":\"string\",\"default\":\"Hi\"}]"),error));
        assert(enable("luau.status",true,error));frame(1);
        {auto v=view("luau.status");assert(v.active&&v.enabled&&!v.restart_required&&v.status.find("Hi")!=std::string::npos);}
        assert(configure("luau.status","label",mods::json::Value("Yo"),error));frame(2);
        {auto v=view("luau.status");assert(v.active&&v.status.find("Yo")!=std::string::npos&&v.status.find("Hi")==std::string::npos);}
        assert(enable("luau.status",false,error));frame(3);assert(remove("luau.status",error));

        assert(install(luau_pack("luau.loaded",
            "local nsmbu=require(\"nsmbu\")\n"
            "function nsmbu.hooks.on_loaded()\n"
            "  nsmbu.status(\"loaded\")\n"
            "end\n",""),error));
        assert(enable("luau.loaded",true,error));frame(4);
        {auto v=view("luau.loaded");assert(v.active&&v.status=="loaded");}
        assert(enable("luau.loaded",false,error));frame(5);assert(remove("luau.loaded",error));

        assert(install(luau_pack("luau.boom",
            "local nsmbu=require(\"nsmbu\")\n"
            "function nsmbu.hooks.on_logic_step(step)\n"
            "  error(\"boom\")\n"
            "end\n",""),error));
        assert(install(luau_pack("luau.ok",
            "local nsmbu=require(\"nsmbu\")\n"
            "function nsmbu.hooks.on_logic_step(step)\n"
            "  nsmbu.status(\"ok\")\n"
            "end\n",""),error));
        assert(enable("luau.boom",true,error));assert(enable("luau.ok",true,error));frame(10);
        {auto a=view("luau.boom");assert(!a.active&&!a.enabled&&a.reason.find("boom")!=std::string::npos);}
        {auto b=view("luau.ok");assert(b.active&&b.enabled&&b.status=="ok");}
        assert(enable("luau.ok",false,error));frame(11);assert(remove("luau.boom",error));assert(remove("luau.ok",error));

        assert(install(luau_pack("luau.sandbox",
            "assert(io==nil)\n"
            "assert(os==nil)\n"
            "assert(type(coroutine)==\"table\")\n"
            "assert(type(utf8)==\"table\")\n"
            "assert(type(rawget(_G,\"nsmbu\"))==\"table\")\n"
            "local nsmbu=require(\"nsmbu\")\n"
            "local guest=require(\"nsmbu.guest\")\n"
            "assert(type(guest.read_u32)==\"function\")\n"
            "local sib=require(\"sib\")\n"
            "function nsmbu.hooks.on_logic_step(step)\n"
            "  nsmbu.status(sib.tag)\n"
            "end\n","",
            "return {tag=\"sib-ok\"}\n"),error));
        assert(enable("luau.sandbox",true,error));frame(20);
        {auto v=view("luau.sandbox");assert(v.active&&v.status=="sib-ok");}
        assert(enable("luau.sandbox",false,error));frame(21);assert(remove("luau.sandbox",error));

        assert(install(luau_pack("luau.escape",
            "require(\"../escape\")\n",""),error));
        assert(enable("luau.escape",true,error));frame(22);
        {auto v=view("luau.escape");assert(!v.active&&!v.enabled&&!v.reason.empty());}
        assert(remove("luau.escape",error));

        assert(install(luau_pack("luau.dots",
            "require(\"foo..bar\")\n",""),error));
        assert(enable("luau.dots",true,error));frame(23);
        {auto v=view("luau.dots");assert(!v.active&&!v.enabled);}
        assert(remove("luau.dots",error));

        assert(install(luau_pack("luau.hostmiss",
            "require(\"nsmbu.missing\")\n",""),error));
        assert(enable("luau.hostmiss",true,error));frame(24);
        {auto v=view("luau.hostmiss");assert(!v.active&&!v.enabled);}
        assert(remove("luau.hostmiss",error));

        std::vector<uint8_t> slab(65536,0);
        mods::luau_guest::set_base(reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(slab.data())-0x10000000u));
        assert(install(luau_pack("luau.guest",
            "local nsmbu=require(\"nsmbu\")\n"
            "local guest=require(\"nsmbu.guest\")\n"
            "function nsmbu.hooks.on_logic_step(step)\n"
            "  local addr=0x10000000\n"
            "  local ok=guest.write_u32(addr,0x11223344)\n"
            "  local v=guest.read_u32(addr)\n"
            "  local bad=guest.read_u32(0x50)\n"
            "  local big=guest.read(addr,0x200000)\n"
            "  if ok and v==0x11223344 and bad==nil and big==nil then\n"
            "    nsmbu.status(\"mem-ok\")\n"
            "  else\n"
            "    nsmbu.status(\"mem-fail\")\n"
            "  end\n"
            "end\n",""),error));
        assert(enable("luau.guest",true,error));frame(30);
        {auto v=view("luau.guest");assert(v.active&&v.status=="mem-ok");}
        assert(slab[0]==0x11&&slab[1]==0x22&&slab[2]==0x33&&slab[3]==0x44);
        assert(enable("luau.guest",false,error));frame(31);assert(remove("luau.guest",error));
        mods::luau_guest::set_base(nullptr);

        std::vector<uint8_t> game_slab(0x200000,0);
        mods::luau_guest::set_base(reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(game_slab.data())-0x10000000u));
        {
            std::string err;
            const uint32_t timer=0x10001000u,field=0x10002000u,mgr=0x10003000u,actor=0x10004000u;
            assert(mods::luau_guest::write_u32(0x101D15F4u,timer,err));
            assert(mods::luau_guest::write_u32(0x101D1604u,field,err));
            assert(mods::luau_guest::write_u32(0x101E6994u,mgr,err));
            assert(mods::luau_guest::write_u32(timer+0x14u,100u<<12,err));
            assert(mods::luau_guest::write_u8(field+0x24u,1,err));
            assert(mods::luau_guest::write_s32(field+0x24u+0x4u,5,err));
            assert(mods::luau_guest::write_s32(field+0x24u+0x8u,2,err));
            assert(mods::luau_guest::write_s32(field+0x24u+0x34u,7,err));
            assert(mods::luau_guest::write_u32(mgr+0x20u,actor,err));
            assert(mods::luau_guest::write_s32(actor+0x500u,2,err));
            assert(mods::luau_guest::write_f32(actor+0x6Cu,1.0f,err));
            assert(mods::luau_guest::write_f32(actor+0x70u,2.0f,err));
            assert(mods::luau_guest::write_f32(actor+0x74u,3.0f,err));
            assert(mods::luau_guest::write_f32(actor+0x78u,0.5f,err));
            assert(mods::luau_guest::write_f32(actor+0x7Cu,3.0f,err));
            assert(mods::luau_guest::write_f32(actor+0x80u,-0.25f,err));
            assert(mods::luau_guest::write_u32(actor+0x1AC8u,1u,err));
            assert(mods::luau_guest::write_s32(field+0x144u,40,err));
            assert(mods::luau_guest::write_u32(field+0x148u,1000u,err));
        }
        assert(install(luau_pack("luau.game",
            "local nsmbu=require(\"nsmbu\")\n"
            "local level=require(\"nsmbu.level\")\n"
            "local player=require(\"nsmbu.player\")\n"
            "function nsmbu.hooks.on_logic_step(step)\n"
            "  if not level.active() then nsmbu.status('no-course') return end\n"
            "  level.set_time(999)\n"
            "  level.set_coins(88)\n"
            "  level.set_score(12345)\n"
            "  local p=player.get_player(0)\n"
            "  if not p or not p:entered() then nsmbu.status('no-player') return end\n"
            "  p:set_lives(99)\n"
            "  p:set_mode(3)\n"
            "  p:set_coins(12)\n"
            "  p:set_speed_y(6.5)\n"
            "  p:set_pos(10,20,30)\n"
            "  p:set_speed({x=1.5,y=6.5,z=-2})\n"
            "  local pos=p:pos()\n"
            "  local spd=p:speed()\n"
            "  local n=0\n"
            "  for q in player.get_players() do n=n+1 end\n"
            "  if level.time()==999 and level.coins()==88 and level.score()==12345\n"
            "      and p:lives()==99 and p:mode()==3 and p:coins()==12 and p:speed_y()==6.5 and n==1\n"
            "      and pos and pos.x==10 and pos.y==20 and pos.z==30\n"
            "      and spd and spd.x==1.5 and spd.y==6.5 and spd.z==-2 and p:grounded() then\n"
            "    nsmbu.status('game-ok')\n"
            "  else\n"
            "    nsmbu.status('game-fail')\n"
            "  end\n"
            "end\n",""),error));
        assert(enable("luau.game",true,error));frame(32);
        {auto v=view("luau.game");assert(v.active&&v.status=="game-ok");}
        {
            std::string err;
            uint32_t units=0;assert(mods::luau_guest::read_u32(0x10001000u+0x14u,units,err));assert(units==(999u<<12));
            int32_t lives=0;assert(mods::luau_guest::read_s32(0x10002000u+0x24u+0x4u,lives,err));assert(lives==99);
            int32_t mode=0;assert(mods::luau_guest::read_s32(0x10002000u+0x24u+0x8u,mode,err));assert(mode==3);
            int32_t actor_mode=0;assert(mods::luau_guest::read_s32(0x10004000u+0x500u,actor_mode,err));assert(actor_mode==3);
            int32_t coins=0;assert(mods::luau_guest::read_s32(0x10002000u+0x24u+0x34u,coins,err));assert(coins==12);
            float speed=0;assert(mods::luau_guest::read_f32(0x10004000u+0x7Cu,speed,err));assert(speed==6.5f);
            float px=0,py=0,pz=0;
            assert(mods::luau_guest::read_f32(0x10004000u+0x6Cu,px,err));assert(px==10.0f);
            assert(mods::luau_guest::read_f32(0x10004000u+0x70u,py,err));assert(py==20.0f);
            assert(mods::luau_guest::read_f32(0x10004000u+0x74u,pz,err));assert(pz==30.0f);
            float sx=0,sz=0;
            assert(mods::luau_guest::read_f32(0x10004000u+0x78u,sx,err));assert(sx==1.5f);
            assert(mods::luau_guest::read_f32(0x10004000u+0x80u,sz,err));assert(sz==-2.0f);
            int32_t course_coins=0;assert(mods::luau_guest::read_s32(0x10002000u+0x144u,course_coins,err));assert(course_coins==88);
            uint32_t score=0;assert(mods::luau_guest::read_u32(0x10002000u+0x148u,score,err));assert(score==12345u);
        }
        assert(enable("luau.game",false,error));frame(33);assert(remove("luau.game",error));
        mods::luau_guest::set_base(nullptr);

        auto content_only=root/"luau.files";fs::create_directories(content_only/"content"/"Common");
        std::ofstream(content_only/"content"/"Common"/"a.bin")<<"a";
        std::ofstream(content_only/"manifest.json")<<R"({"format_version":2,"id":"luau.files","name":"Files","version":"1.0.0","game_id":"nsmbu-usa","kind":"luau","content_dir":"content"})";
        assert(install(content_only.string(),error));
        {auto v=view("luau.files");assert(v.kind=="luau"&&v.restart_required&&!v.active);}
        assert(enable("luau.files",true,error));frame(40);
        {auto v=view("luau.files");assert(v.enabled&&!v.active&&v.pending_restart&&v.restart_required);}

        auto content_b=root/"luau.files2";fs::create_directories(content_b/"content"/"Common");
        std::ofstream(content_b/"content"/"Common"/"a.bin")<<"b";
        std::ofstream(content_b/"manifest.json")<<R"({"format_version":2,"id":"luau.files2","name":"Files2","version":"1.0.0","game_id":"nsmbu-usa","kind":"luau","content_dir":"content"})";
        assert(install(content_b.string(),error));
        assert(!enable("luau.files2",true,error));assert(error.find("Content file conflict")!=std::string::npos);
        assert(enable("luau.files",false,error));frame(41);assert(remove("luau.files",error));assert(remove("luau.files2",error));

        auto both=root/"luau.both";fs::create_directories(both/"content"/"Common");
        std::ofstream(both/"content"/"Common"/"b.bin")<<"x";
        std::ofstream(both/"main.luau")<<"function nsmbu.hooks.on_logic_step(step) nsmbu.status(\"both\") end\n";
        std::ofstream(both/"manifest.json")<<R"({"format_version":2,"id":"luau.both","name":"Both","version":"1.0.0","game_id":"nsmbu-usa","kind":"luau","luau":{"api_version":1,"entry":"main.luau"},"content_dir":"content"})";
        assert(install(both.string(),error));
        assert(enable("luau.both",true,error));frame(42);
        {auto v=view("luau.both");assert(v.enabled&&!v.active&&v.pending_restart&&v.restart_required);}
        assert(enable("luau.both",false,error));frame(43);assert(remove("luau.both",error));

        assert(!hot_reload());
        assert(install(luau_pack("luau.reload",
            "local nsmbu=require(\"nsmbu\")\n"
            "function nsmbu.hooks.on_logic_step(step)\n"
            "  nsmbu.status(\"before\")\n"
            "end\n",""),error));
        assert(enable("luau.reload",true,error));frame(50);
        {auto v=view("luau.reload");assert(v.active&&v.status=="before");}
        assert(set_hot_reload(true,error));assert(hot_reload());
        frame(51);
        {
            auto script=storage/"mods"/"luau.reload"/"main.luau";
            std::ofstream(script)
                <<"local nsmbu=require(\"nsmbu\")\n"
                <<"function nsmbu.hooks.on_logic_step(step)\n"
                <<"  nsmbu.status(\"after\")\n"
                <<"end\n";
            auto stamp=fs::file_time_type::clock::now()+std::chrono::seconds(2);
            fs::last_write_time(script,stamp);
        }
        frame(70);frame(90);
        {auto v=view("luau.reload");assert(v.active&&v.enabled&&v.status=="after");}
        {
            auto script=storage/"mods"/"luau.reload"/"main.luau";
            std::ofstream(script)<<"this is not valid luau {{\n";
            auto stamp=fs::file_time_type::clock::now()+std::chrono::seconds(4);
            fs::last_write_time(script,stamp);
        }
        frame(110);frame(130);
        {auto v=view("luau.reload");assert(!v.active&&!v.enabled&&!v.reason.empty());}
        assert(remove("luau.reload",error));
        assert(set_hot_reload(false,error));assert(!hot_reload());

        assert(install(both.string(),error));
        assert(enable("luau.both",true,error));
        assert(set_hot_reload(true,error));
        frame(60);
        {
            auto script=storage/"mods"/"luau.both"/"main.luau";
            std::ofstream(script)
                <<"local nsmbu=require(\"nsmbu\")\n"
                <<"function nsmbu.hooks.on_logic_step(step)\n"
                <<"  nsmbu.status(\"hot-both\")\n"
                <<"end\n";
            auto stamp=fs::file_time_type::clock::now()+std::chrono::seconds(2);
            fs::last_write_time(script,stamp);
        }
        frame(61);frame(62);
        {auto v=view("luau.both");assert(v.enabled&&!v.active&&v.pending_restart&&v.restart_required);}
        assert(enable("luau.both",false,error));frame(63);
        assert(set_hot_reload(false,error));
        assert(remove("luau.both",error));

        fs::remove_all(root);
        std::cout<<"Luau load, options, fault isolation, sandbox, guest memory, level/player API, content_dir, and hot reload passed\n";
        return 0;
    }
    if(argc==2&&(std::string(argv[1])=="--cemu-startup"||std::string(argv[1])=="--cemu-backend")){
        bool backend=std::string(argv[1])=="--cemu-backend";
        auto root=fs::temp_directory_path()/("nsmbu-cemu-startup-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        auto storage=root/"storage",pack=storage/"mods"/"cemu.test";fs::create_directories(pack);
        std::ofstream(pack/"manifest.json")<<R"({"format_version":1,"id":"cemu.test","name":"Test","version":"1.0.0","game_id":"nsmbu-usa","kind":"cemu","cemu_dir":""})";
        std::ofstream(pack/"rules.txt")<<"[Definition]\nname=Test\ntitleIds=0005000010101d00\nversion=4\n[Preset]\nname=Normal\n$scale=1\n[Preset]\nname=Double\n$scale=2\n[TextureRedefine]\nwidth=1280\nheight=720\noverwriteWidth=1280*$scale\n";
        std::ofstream(storage/"profiles.json")<<R"({"format_version":1,"active":"Default","profiles":{"Default":{"enabled":{"cemu.test":true},"config":{"cemu.test":{"preset-0":"Double"}}}}})";
        if(backend){
            std::ofstream(pack/"0000000000000001_0000000000000002_ps.txt")<<"#version 420\nvoid main(){}\n";
            auto content=storage/"mods"/"content.test";fs::create_directories(content/"content"/"Common");
            std::ofstream(content/"content"/"Common"/"test.bin")<<"synthetic content";
            std::ofstream(content/"manifest.json")<<R"({"format_version":2,"id":"content.test","name":"Content","version":"1.0.0","game_id":"nsmbu-usa","kind":"luau","content_dir":"content"})";
            std::ofstream(storage/"profiles.json")<<R"({"format_version":1,"active":"Default","profiles":{"Default":{"enabled":{"cemu.test":true,"content.test":true}}}})";
        }
        env("NSMBU_NO_HOST_INPUT","1");env("NSMBU_MOD_MANAGER_DIR",storage.string().c_str());initialize();
        if(backend){
            assert(!mods::content::replacement("Common/test.bin").empty());
            for(const auto& view:list())if(view.id=="cemu.test")assert(!view.active&&!view.compatible&&view.enabled);
            std::string error;assert(enable("cemu.test",false,error));assert(remove("cemu.test",error));
            assert(!mods::content::replacement("Common/test.bin").empty());fs::remove_all(root);
            std::cout<<"Unavailable shader backend preserves content and permits disabling shader packs\n";return 0;
        }
        uint32_t width=0,height=0;assert(mods::cemu::texture_extent(1280,720,0x80e,1,4,width,height)&&width==2560&&height==720);
        std::string error;assert(list().at(0).active&&!list().at(0).pending_restart);
        assert(configure("cemu.test","preset-0","Normal",error));assert(list().at(0).pending_restart);
        assert(mods::cemu::texture_extent(1280,720,0x80e,1,4,width,height)&&width==2560);
        assert(enable("cemu.test",false,error));frame(100);assert(list().at(0).active&&list().at(0).pending_restart);
        assert(!remove("cemu.test",error));assert(!install(pack.string(),error));fs::remove_all(root);
        std::cout<<"Cemu startup presets and restart-only immutable lifecycle passed\n";return 0;
    }
    if(argc==2&&std::string(argv[1])=="--content-startup"){
        auto root=fs::temp_directory_path()/("nsmbu-content-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        auto storage=root/"storage";auto pack=storage/"mods"/"content.test";
        fs::create_directories(pack/"content"/"Common");
        std::ofstream(pack/"content"/"Common"/"fixture.bin")<<"synthetic replacement";
        fs::create_directories(pack/"content"/"Common"/"Pack");std::ofstream(pack/"content"/"Common"/"Pack"/"permanent_2d_EuEnglish.pack")<<"SARCsynthetic translation";
        std::ofstream(pack/"manifest.json")<<R"({"format_version":2,"id":"content.test","name":"Test","version":"1.0.0","game_id":"nsmbu-usa","kind":"luau","content_dir":"content"})";
        std::ofstream(storage/"profiles.json")<<R"({"format_version":1,"active":"Default","profiles":{"Default":{"enabled":{"content.test":true}}}})";
        env("NSMBU_NO_HOST_INPUT","1");env("NSMBU_MOD_MANAGER_DIR",storage.string().c_str());
        assert(mods::content::replacement("/vol/content/Common/fixture.bin").empty());initialize();
        auto file=mods::content::replacement("/vol/content/common/FIXTURE.bin");assert(file==(pack/"content"/"Common"/"fixture.bin").string());
        assert(mods::content::replacement("Common/fixture.bin")==file);
        for(auto path:{"/vol/save/Common/fixture.bin","/vol/code/Common/fixture.bin","/vol/contentX/Common/fixture.bin","/vol/content/../Common/fixture.bin","Common/../Common/fixture.bin","Common\\fixture.bin"})assert(mods::content::replacement(path).empty());
        for(auto mode:{"w","a","r+","r+b","wb"})assert(mods::content::replacement("Common/fixture.bin",mode).empty());
        assert(mods::content::replacement("Common/absent.bin").empty());
        auto translation=(pack/"content"/"Common"/"Pack"/"permanent_2d_EuEnglish.pack").string();
        assert(mods::content::replacement("/vol/content/Common/Pack/permanent_2d_EuEnglish.pack")==translation);
        assert(mods::content::replacement("/vol/content/Common/Pack/permanent_2d_UsEnglish.pack")==translation);
        for(auto other:{"permanent_2d_UsFrench.pack","permanent_2d_EuGerman.pack","permanent_2d_JpJapanese.pack","permanent_3d.pack","permanent_2d_UsEnglish.pack.bak"})
            assert(mods::content::replacement(std::string("/vol/content/Common/Pack/")+other).empty());
        assert(mods::content::replacement("/vol/content/Common/Layout/permanent_2d_UsEnglish.pack").empty());
        assert(mods::content::replacement("/vol/content/Cafe/JP/Pack/permanent_2d_EuEnglish.pack")==translation);
        assert(mods::content::replacement("/vol/content/Cafe/JP/Pack/permanent_2d_EuGerman.pack").empty());
        assert(mods::content::replacement("/vol/content/Cafe/JP/Packs/permanent_2d_EuEnglish.pack").empty());
        assert(mods::content::replacement("/vol/content/Common/Pack/permanent_2d_UsEnglish.pack","wb").empty());
        std::string error;assert(list().at(0).kind=="luau");assert(list().at(0).active&&list().at(0).restart_required);assert(enable("content.test",false,error));frame(100);
        assert(list().at(0).active&&!list().at(0).enabled);assert(mods::content::replacement("Common/fixture.bin")==file);
        assert(!remove("content.test",error));assert(!install(pack.string(),error));
        assert(create_profile("Other",error));assert(select_profile("Other",error));frame(101);assert(mods::content::replacement("Common/fixture.bin")==file);
        fs::remove_all(root);std::cout<<"Startup overrides, read-only routing, boundaries, and restart lifecycle passed\n";return 0;
    }
    assert(argc == 2);
    auto root=fs::path(argv[1])/std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    assert(!fs::exists(root));
    fs::create_directories(root);
    env("NSMBU_NO_HOST_INPUT","1");
    env("NSMBU_MOD_MANAGER_DIR",(root/"storage").string().c_str());
    fs::create_directories(root/"storage");
    std::ofstream(root/"storage"/"profiles.json")<<R"({"format_version":1,"active":"Default","profiles":{"Default":{"enabled":{},"builtins":{"direct-camera":true},"builtin_options":{"direct-camera.speed":1.5}}}})";
    auto migrate=root/"storage"/"mods"/"content.migrate";fs::create_directories(migrate/"content"/"Common");
    std::ofstream(migrate/"content"/"Common"/"m.bin")<<"m";
    std::ofstream(migrate/"manifest.json")<<R"({"format_version":1,"id":"content.migrate","name":"Migrate","version":"1.0.0","game_id":"nsmbu-usa","kind":"content","content_dir":"content"})";
    initialize();
    std::string error;
    {auto v=view("content.migrate");assert(v.kind=="luau"&&!v.enabled&&!v.active&&v.restart_required);
        std::ifstream mf(migrate/"manifest.json");std::string text{std::istreambuf_iterator<char>(mf),{}};
        assert(text.find("\"kind\":\"luau\"")!=std::string::npos);assert(text.find("\"kind\":\"content\"")==std::string::npos);}
    assert(remove("content.migrate",error));
    auto content_package=[&](const char* id) {
        auto path=root/id;fs::create_directories(path/"content"/"Common");
        std::ofstream(path/"content"/"Common"/(std::string(id)+".bin"))<<"x";
        std::ofstream(path/"manifest.json") << "{\"format_version\":2,\"id\":\"" << id
          << "\",\"name\":\"" << id << "\",\"version\":\"1.0.0\",\"game_id\":\"nsmbu-usa\","
          << "\"kind\":\"luau\",\"content_dir\":\"content\"}";
        return path.string();
    };
    auto cemu_package=[&](const char* id, const char* extra) {
        auto path=root/id;fs::create_directories(path);
        std::ofstream(path/"rules.txt")<<("[Definition]\nname="+std::string(id)+"\ntitleIds=0005000010101d00\nversion=4\n[Preset]\nname=Normal\n$scale=1\n[Preset]\nname=Double\n$scale=2\n[TextureRedefine]\nwidth=1280\nheight=720\noverwriteWidth=1280*$scale\n");
        std::ofstream(path/"manifest.json") << "{\"format_version\":1,\"id\":\"" << id
          << "\",\"name\":\"" << id << "\",\"version\":\"1.0.0\",\"game_id\":\"nsmbu-usa\","
          << "\"kind\":\"cemu\",\"cemu_dir\":\"\"" << extra << "}";
        return path.string();
    };
    auto rejected=root/"rejected";fs::create_directories(rejected);
    std::ofstream(rejected/"rules.txt")<<"[Definition]\nname=Rejected\ntitleIds=0005000010101d00\nversion=4\n[TextureRedefine]\nwidth=1280\nheight=720\noverwriteWidth=1280\n";
    std::ofstream(rejected/"manifest.json")<<R"({"format_version":1,"id":"rejected","name":"Rejected","version":"1.0.0","game_id":"nsmbu-usa","kind":"cemu","cemu_dir":"","dependencies":[{"id":"builtin:direct-camera"}]})";
    assert(!install(rejected.string(),error));
    assert(error.find("Unknown built-in mod: builtin:direct-camera")!=std::string::npos);
    for(const auto& [kind,message]:std::initializer_list<std::pair<const char*,const char*>>{
            {"native","Native mods are not supported"},
            {"guest","Guest mods are not supported"},
            {"settings","Settings mods are not supported"}}){
        auto rejected_kind=root/(std::string("rejected-")+kind);fs::create_directories(rejected_kind);
        std::ofstream(rejected_kind/"manifest.json")<<"{\"format_version\":1,\"id\":\"rejected-"<<kind<<"\",\"name\":\"Rejected\",\"version\":\"1.0.0\",\"game_id\":\"nsmbu-usa\",\"kind\":\""<<kind<<"\"}";
        assert(!install(rejected_kind.string(),error));
        assert(error.find(message)!=std::string::npos);
    }
    assert(install(content_package("climb-preset"),error));
    assert(list().size()==1 && list()[0].id=="climb-preset");
    assert(list()[0].kind=="luau" && list()[0].restart_required);
    assert(enable("climb-preset",true,error));frame(1);
    assert(view("climb-preset").enabled && !view("climb-preset").active && view("climb-preset").pending_restart);
    {
        std::ifstream saved(root/"storage"/"profiles.json");
        std::string text{std::istreambuf_iterator<char>(saved),{}};
        assert(text.find("builtins")==std::string::npos);
        assert(text.find("builtin_options")==std::string::npos);
    }
    assert(!remove("climb-preset",error));
    assert(enable("climb-preset",false,error));frame(2);assert(!view("climb-preset").enabled);
    assert(create_profile("Adventure",error));
    assert(enable("climb-preset",true,error));frame(20);assert(view("climb-preset").enabled);
    assert(select_profile("Adventure",error));frame(21);assert(!view("climb-preset").enabled);
    assert(current_profile()=="Adventure");assert(!delete_profile("Adventure",error));
    assert(select_profile("Default",error));assert(delete_profile("Adventure",error));
    assert(install(cemu_package("missing-dep",",\"dependencies\":[{\"id\":\"absent\"}]"),error));
    assert(!enable("missing-dep",true,error));
    assert(remove("missing-dep",error));
    assert(install(cemu_package("cycle-a",",\"dependencies\":[{\"id\":\"cycle-b\"}]"),error));
    assert(install(cemu_package("cycle-b",",\"dependencies\":[{\"id\":\"cycle-a\"}]"),error));
    assert(!enable("cycle-a",true,error));assert(error.find("cycle")!=std::string::npos);
    assert(remove("cycle-a",error));assert(remove("cycle-b",error));
    assert(install(cemu_package("conflicting",",\"conflicts\":[\"climb-preset\"]"),error));
    assert(!enable("conflicting",true,error));assert(remove("conflicting",error));
    assert(install(cemu_package("typed",""),error));
    assert(configure("typed","preset-0","Double",error));assert(!configure("typed","preset-0","Unknown",error));
    assert(install((root/"typed").string(),error));assert(remove("typed",error));
    auto bad=root/"bad.nsmbumod";std::ofstream(bad)<<"not a package";assert(!install(bad.string(),error));
    assert(enable("climb-preset",false,error));frame(30);assert(remove("climb-preset",error));
    auto plain=root/"plain";fs::create_directories(plain/"content"/"Common");
    std::ofstream(plain/"content"/"Common"/"a.bin")<<"x";
    std::ofstream(plain/"manifest.json")<<R"({"format_version":2,"id":"plain","name":"Plain","version":"1.0.0","game_id":"nsmbu-usa","kind":"luau","content_dir":"content"})";
    assert(install(plain.string(),error));
    assert(view("plain").kind=="luau");
    assert(remove("plain",error));
    assert(list().empty());
    auto graphics=root/"CemuResolution";fs::create_directories(graphics);
    std::ofstream(graphics/"rules.txt")<<"[Definition]\nname=Resolution\ntitleIds=0005000010101d00\nversion=4\n[Preset]\nname=Normal\n$scale=1\n[Preset]\nname=Double\n$scale=2\n[TextureRedefine]\nwidth=1280\nheight=720\noverwriteWidth=1280*$scale\noverwriteHeight=720*$scale\n";
    assert(install(graphics.string(),error));auto graphicsView=list().at(0);
    assert(graphicsView.kind=="cemu"&&graphicsView.restart_required&&graphicsView.options.size()==1);
    assert(configure(graphicsView.id,"preset-0","Double",error));
    assert(!configure(graphicsView.id,"preset-0","Unknown",error));
    assert(enable(graphicsView.id,true,error));frame(45);
    assert(!list().at(0).active&&list().at(0).pending_restart);
    assert(enable(graphicsView.id,false,error));assert(remove(graphicsView.id,error));
    std::ofstream(graphics/"0000000000000001_0000000000000002_ps.txt")<<"#version 420\nvoid main(){}\n";
    assert(install(graphics.string(),error));assert(!list().at(0).compatible);
    assert(!enable(list().at(0).id,true,error));assert(remove(list().at(0).id,error));
    auto legacy=root/"LegacyModel";fs::create_directories(legacy/"content"/"Object");
    std::ofstream(legacy/"content"/"Object"/"test.arc")<<"synthetic model archive";
    assert(install(legacy.string(),error));auto imported=list().at(0);assert(imported.id=="content.legacymodel"&&imported.kind=="luau"&&imported.restart_required&&!imported.active);
    assert(enable(imported.id,true,error));frame(50);assert(!list().at(0).active);
    auto second=root/"OtherModel";fs::create_directories(second/"content"/"Object");std::ofstream(second/"content"/"Object"/"test.arc")<<"synthetic conflicting archive";
    assert(install(second.string(),error));assert(!enable("content.othermodel",true,error));assert(error.find("Content file conflict")!=std::string::npos);
    assert(enable(imported.id,false,error));frame(51);assert(remove(imported.id,error));assert(remove("content.othermodel",error));
    auto loose=root/"permanent_3d.pack";std::ofstream(loose)<<"SARCsynthetic-fixture";
    assert(install(loose.string(),error));assert(list().at(0).id=="content.permanent_3d");assert(remove(list().at(0).id,error));
    auto loose_folder=root/"LooseModel";fs::create_directories(loose_folder);std::ofstream(loose_folder/"permanent_3d.pack")<<"SARCsynthetic-fixture";
    assert(install(loose_folder.string(),error));assert(remove(list().at(0).id,error));
    std::ofstream(loose_folder/"unknown.pack")<<"SARCsynthetic-fixture";assert(!install(loose_folder.string(),error));
    auto game=root/"game";fs::create_directories(game/"content"/"Common"/"Layout");fs::create_directories(game/"content"/"Common"/"Object");
    std::ofstream(game/"content"/"Common"/"Layout"/"Title_00.szs")<<"original";std::ofstream(game/"content"/"Common"/"Object"/"Twice.szs")<<"a";
    fs::create_directories(game/"content"/"Common"/"Stage");std::ofstream(game/"content"/"Common"/"Stage"/"Twice.szs")<<"b";
    mods::content::set_game_root(game);
    auto translation=root/"FanTranslation";fs::create_directories(translation/"inner");
    std::ofstream(translation/"inner"/"permanent_2d_EuEnglish.pack")<<"SARCsynthetic translation";std::ofstream(translation/"Title_00.szs")<<"Yaz0logo";
    std::ofstream(translation/"readme.txt")<<"text";
    assert(install(translation.string(),error));{auto v=list().at(0);assert(v.id=="content.fantranslation");
        assert(v.description.find("Common/Pack/permanent_2d_EuEnglish.pack")!=std::string::npos&&v.description.find("Common/Layout/Title_00.szs")!=std::string::npos);
        assert(v.description.find("Not used: readme.txt")!=std::string::npos);assert(remove(v.id,error));}
    std::ofstream(translation/"Twice.szs")<<"ambiguous";assert(!install(translation.string(),error));fs::remove(translation/"Twice.szs");
    auto single=root/"permanent_2d_JpJapanese.pack";std::ofstream(single)<<"SARCsynthetic";assert(install(single.string(),error));assert(remove(list().at(0).id,error));
    fs::create_directories(translation/"content"/"Common"/"Pack");fs::rename(translation/"inner"/"permanent_2d_EuEnglish.pack",translation/"content"/"Common"/"Pack"/"permanent_2d_EuEnglish.pack");
    assert(install((translation/"content").string(),error));assert(list().at(0).id=="content.fantranslation");assert(remove(list().at(0).id,error));
    mods::content::set_game_root({});
    auto invalid=root/"CodeMod";fs::create_directories(invalid/"content");std::ofstream(invalid/"content"/"dummy")<<"fixture";std::ofstream(invalid/"patches.txt")<<"code";assert(!install(invalid.string(),error));
    fs::remove(invalid/"patches.txt");fs::create_directories(invalid/"graphicPacks"/"Patch");std::ofstream(invalid/"graphicPacks"/"Patch"/"rules.txt")<<"[Definition]\ntitleIds = 0005000010101d00\n";
    std::ofstream(invalid/"graphicPacks"/"Patch"/"patch_code.asm")<<"[Code]\nmoduleMatches = 0x475BD29F\n";assert(!install(invalid.string(),error));assert(error.find("code patch")!=std::string::npos);
    fs::remove_all(invalid/"graphicPacks");std::ofstream(invalid/"rules.txt")<<"[Definition]\ntitleIds = 0005000010101e00\n";assert(!install(invalid.string(),error));
    std::ofstream(invalid/"rules.txt",std::ios::trunc)<<"[Definition]\ntitleIds = 0005000010101d00\n[TextureRedefine]\n";assert(!install(invalid.string(),error));
    fs::remove(invalid/"rules.txt");std::ofstream(invalid/"content"/".deleted_dummy")<<"";assert(!install(invalid.string(),error));
    auto duplicate=root/"Duplicate";fs::create_directories(duplicate/"content"/"Object");fs::create_directories(duplicate/"content"/"object");
    std::ofstream(duplicate/"content"/"Object"/"A.bin")<<"fixture";std::ofstream(duplicate/"content"/"object"/"a.bin")<<"fixture";
    if(std::distance(fs::directory_iterator(duplicate/"content"),fs::directory_iterator{})==2)assert(!install(duplicate.string(),error));

    auto cloudy=root/"storage"/"mods"/"Cloudy Level Pack";
    fs::create_directories(cloudy/"content"/"Common"/"Course");
    fs::create_directories(cloudy/"meta");
    std::ofstream(cloudy/"content"/"Common"/"Course"/"1-1.szs")<<"level";
    std::ofstream(cloudy/"meta"/"iconTex.tga")<<"icon";
    std::ofstream(cloudy/"rules.txt")<<"[Definition]\nname = Cloudy Level Pack\ntitleIds = 0005000010101C00,0005000010101D00\ndescription = Test level pack\nversion = 6\n";
    assert(refresh(error));
    {
        auto v=view("content.cloudy-level-pack");
        assert(v.compatible&&v.kind=="luau"&&v.name=="Cloudy Level Pack"&&v.version=="1.0.0");
        assert(v.description.find("Cemu pack version 6")!=std::string::npos);
        assert(v.restart_required&&!v.enabled);
        std::ifstream mf(cloudy/"manifest.json");std::string text{std::istreambuf_iterator<char>(mf),{}};
        assert(text.find("\"content_dir\":\"content\"")!=std::string::npos);
        assert(text.find("\"kind\":\"luau\"")!=std::string::npos);
    }
    assert(enable("content.cloudy-level-pack",true,error));
    assert(enable("content.cloudy-level-pack",false,error));
    assert(remove("content.cloudy-level-pack",error));

    auto wrong_title=root/"storage"/"mods"/"WrongTitle";
    fs::create_directories(wrong_title/"content"/"Common");
    std::ofstream(wrong_title/"content"/"Common"/"a.bin")<<"x";
    std::ofstream(wrong_title/"rules.txt")<<"[Definition]\nname=Wrong\ntitleIds=0005000010101e00\nversion=6\n";
    assert(refresh(error));
    {auto v=view("WrongTitle");assert(!v.compatible&&v.reason.find("does not target")!=std::string::npos);}
    fs::remove_all(wrong_title);

    auto mixed=root/"storage"/"mods"/"MixedPack";
    fs::create_directories(mixed/"content"/"Common");
    std::ofstream(mixed/"content"/"Common"/"a.bin")<<"x";
    std::ofstream(mixed/"rules.txt")<<"[Definition]\nname=Mixed\ntitleIds=0005000010101d00\nversion=4\n[Preset]\nname=Normal\n$scale=1\n[TextureRedefine]\nwidth=1280\nheight=720\noverwriteWidth=1280\n";
    assert(refresh(error));
    {auto v=view("MixedPack");assert(!v.compatible);assert(v.reason.find("Mixed content")!=std::string::npos||v.reason.find("texture")!=std::string::npos||v.reason.find("adapter")!=std::string::npos||v.reason.find("content")!=std::string::npos);}
    fs::remove_all(mixed);

    auto with_meta=root/"WithMeta";fs::create_directories(with_meta/"content"/"Common");fs::create_directories(with_meta/"meta");
    std::ofstream(with_meta/"content"/"Common"/"a.bin")<<"x";std::ofstream(with_meta/"meta"/"meta.xml")<<"<meta/>";
    std::ofstream(with_meta/"rules.txt")<<"[Definition]\nname=With Meta\ntitleIds=0005000010101d00\nversion=6\n";
    assert(install(with_meta.string(),error));
    assert(view("content.with-meta").kind=="luau");
    assert(remove("content.with-meta",error));

    auto large_dir=root/"LargeFile";fs::create_directories(large_dir/"content");
    auto large=large_dir/"content"/"big.bin";
    {std::ofstream(large)<<"x";}
    fs::resize_file(large,129ull*1024*1024);
    auto indexed=mods::content::index(large_dir/"content");
    assert(indexed.size()==1);
    fs::remove_all(large_dir);

    std::cout << "Package install, profiles, dependencies, content and cemu lifecycle passed\n";
}
