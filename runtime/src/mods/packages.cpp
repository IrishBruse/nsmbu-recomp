#include "packages.h"
#include "mod_archive.h"
#include "content.h"
#include "cemu_pack.h"
#include "lua_runtime.h"
#include "../platform/host.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>

namespace mods::packages {
namespace {
namespace fs=std::filesystem;
using json::Value;
struct Requirement {std::string id,version;};
struct Manifest {
    std::string id,name,version,author,description,kind,entry,problem;
    std::vector<Requirement> dependencies;
    std::vector<std::string> conflicts;
    std::vector<Option> options;
    content::Files files;
    std::shared_ptr<cemu::Pack> graphics;
};
struct Record {Manifest manifest;fs::path path;bool active=false,loading=false;std::string status,error;Value startup_config;};
std::mutex mutex;
std::map<std::string,Record> records;
Value database;
fs::path root;
bool ready=false;
std::string last_problem;
std::atomic<bool> dirty{false},running{false},profile_changed{false};
void require(bool ok,const std::string& message){if(!ok)throw std::runtime_error(message);}
bool id_ok(const std::string& s){return !s.empty()&&s.size()<=64&&s[0]!='.'&&std::all_of(s.begin(),s.end(),[](unsigned char c){return (c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'||c=='-'||c=='.';});}
std::array<unsigned,3> version(const std::string& s) {
    std::array<unsigned,3> result{};size_t p=0;
    for(int i=0;i<3;i++){size_t end=i==2?s.size():s.find('.',p);require(end!=std::string::npos&&end>p,"Version must be major.minor.patch");auto parse=std::from_chars(s.data()+p,s.data()+end,result[i]);require(parse.ec==std::errc()&&parse.ptr==s.data()+end,"Invalid version");p=end+1;}
    return result;
}
std::string string_field(const Value& v,const char* key,bool optional=false,size_t limit=8192){const auto& f=v.get(key);if(optional&&f.type==Value::Null)return {};require(f.type==Value::String&&f.text.size()<=limit&&f.text.find('\0')==std::string::npos,"Invalid field: "+std::string(key));return f.text;}

std::string read_text(const fs::path& p){require(fs::is_regular_file(p)&&!fs::is_symlink(p)&&fs::file_size(p)<=1024*1024,"Missing or oversized JSON file: "+p.filename().string());std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
bool valid_option(const Option& o,const Value& v){
    if(o.type=="bool")return v.type==Value::Bool;
    if(o.type=="number")return v.type==Value::Number&&std::isfinite(v.number)&&v.number>=o.minimum&&v.number<=o.maximum;
    if(o.type=="string")return v.type==Value::String&&v.text.size()<=1024&&v.text.find('\0')==std::string::npos;
    if(o.type=="enum")return v.type==Value::String&&std::find(o.choices.begin(),o.choices.end(),v.text)!=o.choices.end();
    return false;
}
void reject_lua_files(const fs::path& root){
    for(const auto& file:fs::recursive_directory_iterator(root)){
        if(!file.is_regular_file()&&!file.is_symlink())continue;
        auto name=file.path().filename().string();
        auto lower=name;for(char& c:lower)if(c>='A'&&c<='Z')c+='a'-'A';
        bool bad=lower.ends_with(".so")||lower.ends_with(".dll")||lower.ends_with(".dylib")||lower.ends_with(".elf")||lower.ends_with(".rpx")||lower=="patches.txt"||lower=="rules.txt"||lower.ends_with("_vs.txt")||lower.ends_with("_ps.txt");
        require(!bad,"Lua package may not contain "+name);
    }
}
void index_content_dir(Manifest& m,const fs::path& path,const std::string& folder){
    require(archive::relative_path(folder),"Invalid content directory");
    auto checked=path;for(const auto& part:fs::path(folder)){checked/=part;require(!fs::is_symlink(checked),"Content paths may not use symlinks");}
    m.files=content::index(path/folder);
}
bool has_content_files(const Manifest& m){return !m.files.empty();}
bool startup_only(const Manifest& m){return m.kind=="cemu"||m.kind=="content"||(m.kind=="lua"&&has_content_files(m));}
bool content_lifecycle(const Manifest& m){return m.kind=="cemu"||m.kind=="content"||has_content_files(m);}
void migrate_content_alias(const fs::path& path){
    auto v=json::parse(read_text(path/"manifest.json"));
    if(v.get("kind").type!=Value::String||v.get("kind").text!="content")return;
    v["kind"]="lua";
    v["format_version"]=2;
    auto tmp=path/"manifest.json.tmp";
    std::ofstream f(tmp,std::ios::binary|std::ios::trunc);
    f<<json::dump(v)<<'\n';
    f.close();
    require(bool(f)&&host::replace_file(tmp.string(),(path/"manifest.json").string()),"Cannot migrate content package");
}
Manifest manifest(const fs::path& path){
    auto v=json::parse(read_text(path/"manifest.json"));require(v.type==Value::Object,"Manifest must be an object");
    Manifest m;m.id=string_field(v,"id",false,64);require(id_ok(m.id),"Invalid mod ID");
    m.name=string_field(v,"name",false,128);require(!m.name.empty(),"Mod name is empty");
    m.version=string_field(v,"version",false,64);version(m.version);
    m.author=string_field(v,"author",true,256);m.description=string_field(v,"description",true);
    m.kind=string_field(v,"kind",false,32);
    require(m.kind!="guest","Guest mods are not supported");
    require(m.kind!="native","Native mods are not supported");
    require(m.kind!="settings","Settings mods are not supported");
    require(m.kind=="content"||m.kind=="cemu"||m.kind=="lua","Unsupported mod kind");
    const auto& fv=v.get("format_version");
    require(fv.type==Value::Number&&((m.kind=="lua"&&(fv.number==1||fv.number==2))||(m.kind!="lua"&&fv.number==1)),"Unsupported manifest format");
    if(string_field(v,"game_id",false,64)!=kGameId)m.problem="This package targets another game";
    auto minimum=string_field(v,"minimum_manager_version",true,64);
    if(!minimum.empty()&&version(minimum)>version(kManagerVersion))m.problem="Requires mod manager "+minimum;
    if(m.kind=="cemu") {
        auto folder=string_field(v,"cemu_dir",false,512);
        require(folder.empty()||archive::relative_path(folder),"Invalid Cemu directory");
        auto checked=path;for(const auto& part:fs::path(folder)){checked/=part;require(!fs::is_symlink(checked),"Cemu paths may not use symlinks");}
        m.graphics=std::make_shared<cemu::Pack>(cemu::parse(path/folder));
        auto schema=cemu::options(*m.graphics);if(v.get("options").type==Value::Null)v["options"]=schema;else require(v.get("options")==schema,"Cemu preset options do not match rules.txt");
    } else if(m.kind=="lua") {
        require(v.get("abi_version").type==Value::Null,"Lua packages do not use abi_version");
        require(v.get("binaries").type==Value::Null,"Lua packages do not use binaries");
        require(v.get("guest").type==Value::Null,"Lua packages do not use guest");
        auto folder=string_field(v,"content_dir",true,512);
        if(!folder.empty())index_content_dir(m,path,folder);
        const auto& lua=v.get("lua");
        if(lua.type==Value::Null){
            require(has_content_files(m),"Lua package requires a script entry or content_dir");
        }else{
            require(lua.type==Value::Object,"Invalid lua object");
            require(lua.get("api_version").type==Value::Number&&lua.get("api_version").number==1,"Unsupported Lua API version");
            m.entry=string_field(lua,"entry",true,512);
            if(!m.entry.empty()){
                require(archive::relative_path(m.entry),"Invalid Lua entry path");
                auto entry_path=path;for(const auto& part:fs::path(m.entry)){entry_path/=part;require(!fs::is_symlink(entry_path),"Lua entry may not use symlinks");}
                require(fs::is_regular_file(entry_path)&&!fs::is_symlink(entry_path),"Lua entry is missing: "+m.entry);
            }
            require(!m.entry.empty()||has_content_files(m),"Lua package requires a script entry or content_dir");
        }
        reject_lua_files(path);
    } else {
        auto folder=string_field(v,"content_dir",false,512);
        index_content_dir(m,path,folder);
    }
    const auto& deps=v.get("dependencies");require(deps.type==Value::Null||deps.type==Value::Array,"Dependencies must be an array");
    for(const auto& dep:deps.array){Requirement r;r.id=string_field(dep,"id",false,80);r.version=string_field(dep,"minimum_version",true,64);if(r.version.empty())r.version="0.0.0";version(r.version);require(!r.id.starts_with("builtin:"),"Unknown built-in mod: "+r.id);require(id_ok(r.id),"Invalid dependency ID");m.dependencies.push_back(r);}
    const auto& conflicts=v.get("conflicts");require(conflicts.type==Value::Null||conflicts.type==Value::Array,"Conflicts must be an array");
    for(const auto& c:conflicts.array){require(c.type==Value::String,"Invalid conflict ID");require(!c.text.starts_with("builtin:"),"Unknown built-in mod: "+c.text);require(id_ok(c.text),"Invalid conflict ID");m.conflicts.push_back(c.text);}
    const auto& options=v.get("options");require(options.type==Value::Null||(options.type==Value::Array&&options.array.size()<=32),"Invalid options");std::set<std::string> option_ids;
    for(const auto& option:options.array){
        Option o;o.id=string_field(option,"id",false,64);require(id_ok(o.id)&&option_ids.insert(o.id).second,"Invalid or duplicate option ID");o.name=string_field(option,"name",false,128);o.description=string_field(option,"description",true);o.type=string_field(option,"type",false,32);o.default_value=option.get("default");
        if(o.type=="number"){const auto& lo=option.get("min");const auto& hi=option.get("max");const auto& step=option.get("step");require(lo.type==Value::Number&&hi.type==Value::Number&&lo.number<=hi.number,"Invalid numeric bounds");o.minimum=lo.number;o.maximum=hi.number;o.step=step.type==Value::Number?step.number:1;require(o.step>0&&std::isfinite(o.step),"Invalid numeric step");}
        if(o.type=="enum"){const auto& choices=option.get("choices");require(choices.type==Value::Array&&!choices.array.empty()&&choices.array.size()<=64,"Invalid enum choices");for(const auto& c:choices.array){require(c.type==Value::String&&c.text.size()<=128,"Invalid enum value");o.choices.push_back(c.text);}}
        require(valid_option(o,o.default_value),"Invalid default for "+o.id);o.value=o.default_value;m.options.push_back(o);
    }
    if(m.kind=="content")require(m.dependencies.empty()&&m.options.empty(),"Content packages do not support dependencies or runtime options yet");
    return m;
}
Value& profile(){return database["profiles"][database.get("active").string("Default")];}
bool wanted(const std::string& id){const auto& v=profile().get("enabled").get(id);return v.type==Value::Bool&&v.boolean;}

Value config(const Manifest& m){Value out;out.type=Value::Object;for(const auto& o:m.options){const auto& saved=profile().get("config").get(m.id).get(o.id);out[o.id]=valid_option(o,saved)?saved:o.default_value;}return out;}
void strip_builtins(){for(auto& entry:database["profiles"].object){entry.second.object.erase("builtins");entry.second.object.erase("builtin_options");}}
void save(){if(!ready)return;strip_builtins();fs::create_directories(root);auto tmp=root/"profiles.json.tmp";std::ofstream f(tmp,std::ios::binary|std::ios::trunc);f<<json::dump(database)<<'\n';f.close();require(bool(f)&&host::replace_file(tmp.string(),(root/"profiles.json").string()),"Cannot save mod profiles");}
void defaults(){database=Value{};database["format_version"]=1;database["active"]="Default";profile()["enabled"].type=Value::Object;}
void scan(){records.clear();if(!ready)return;fs::create_directories(root/"Mods");for(const auto& e:fs::directory_iterator(root/"Mods")){if(!e.is_directory()||e.is_symlink()||!id_ok(e.path().filename().string()))continue;Record r;r.path=e.path();try{migrate_content_alias(e.path());r.manifest=manifest(e.path());require(r.manifest.id==e.path().filename(),"Folder and manifest IDs differ");}catch(const std::exception& ex){r.manifest.id=e.path().filename().string();r.manifest.name=r.manifest.id;r.manifest.problem=ex.what();}auto id=r.manifest.id;records.emplace(id,std::move(r));}}
std::vector<std::string> order(const std::set<std::string>& enabled){
    std::map<std::string,int> mark;std::vector<std::string> result;
    std::function<void(const std::string&)> visit=[&](const std::string& id){require(mark[id]!=1,"Dependency cycle at "+id);if(mark[id]==2)return;mark[id]=1;auto it=records.find(id);require(it!=records.end(),"Missing dependency: "+id);const auto& m=it->second.manifest;require(m.problem.empty(),m.name+": "+m.problem);for(const auto& dep:m.dependencies){auto d=records.find(dep.id);require(d!=records.end(),"Missing dependency: "+dep.id);require(version(d->second.manifest.version)>=version(dep.version),"Dependency "+dep.id+" needs version "+dep.version);require(enabled.contains(dep.id),"Dependency is disabled: "+dep.id);visit(dep.id);}mark[id]=2;result.push_back(id);};
    for(const auto& id:enabled)visit(id);return result;
}
std::set<std::string> enabled_set(){std::set<std::string> result;for(const auto& [id,r]:records)if(wanted(id))result.insert(id);return result;}
void validate_conflicts(const std::set<std::string>& enabled){
    std::map<std::string,std::string> file_owners;
    std::vector<cemu::Selection> graphics;
    for(const auto& id:enabled){const auto& m=records.at(id).manifest;if(m.graphics)graphics.push_back({id,*m.graphics,config(m)});}
    cemu::validate(graphics);
    for(const auto& id:enabled){const auto& m=records.at(id).manifest;for(const auto& [file,path]:m.files){auto [it,inserted]=file_owners.emplace(file,id);require(inserted,"Content file conflict: "+file+" between "+id+" and "+it->second);}for(const auto& conflict:m.conflicts){require(!enabled.contains(conflict),m.name+" conflicts with "+conflict);}}
}
template<class Fn> bool operation(std::string& error,Fn fn){try{std::lock_guard guard(mutex);require(ready,"Mod manager storage is unavailable");fn();error.clear();return true;}catch(const std::exception& e){error=e.what();return false;}}
struct Live {std::unique_ptr<mods::lua_mod::Instance> lua;std::string kind;Value last_config;};
std::map<std::string,Live> live;
std::vector<std::string> live_order;
}


void initialize(){
    std::lock_guard guard(mutex);if(ready)return;const char* override=std::getenv("NSMBU_MOD_MANAGER_DIR");if(std::getenv("NSMBU_NO_HOST_INPUT")&&!override)return;
    root=override?fs::path(override):fs::path(host::config_dir())/"ModManager";ready=true;defaults();
    try{fs::create_directories(root);if(fs::exists(root/"profiles.json")){auto saved=json::parse(read_text(root/"profiles.json"));require(saved.get("format_version").type==Value::Number&&saved.get("format_version").number==1,"Unsupported profile format");require(saved.get("profiles").type==Value::Object&&!saved.get("profiles").object.empty(),"Invalid profiles");require(saved.get("active").type==Value::String&&saved.get("profiles").object.contains(saved.get("active").text),"Invalid active profile");database=std::move(saved);}scan();
        try { auto enabled=enabled_set();auto sequence=order(enabled);validate_conflicts(enabled);
            content::Files files;
            for(const auto& id:sequence){auto& record=records.at(id);if(record.manifest.kind=="content"||(record.manifest.kind=="lua"&&has_content_files(record.manifest))){
                files.insert(record.manifest.files.begin(),record.manifest.files.end());record.active=true;
                record.status=std::to_string(record.manifest.files.size())+" replacement files";
            }}
            std::vector<cemu::Selection> graphics;
            for(const auto& id:sequence){auto& record=records.at(id);if(record.manifest.graphics&&(record.manifest.graphics->shaders.empty()||cemu::vulkan())){
                record.startup_config=config(record.manifest);graphics.push_back({id,*record.manifest.graphics,record.startup_config});
                record.active=true;record.status=std::to_string(record.manifest.graphics->textures.size())+" texture rules, "+std::to_string(record.manifest.graphics->shaders.size())+" shader candidates";
            }}
            cemu::activate(graphics);
            content::activate(std::move(files));
            for(const auto& id:sequence){auto& record=records.at(id);if(record.manifest.kind!="lua"||!has_content_files(record.manifest)||record.manifest.entry.empty())continue;
                Live item;item.kind=record.manifest.kind;try{
                    auto cfg=config(record.manifest);
                    mods::lua_mod::LoadInfo info;info.id=record.manifest.id;info.version=record.manifest.version;info.path=record.path.string();info.entry=record.manifest.entry;info.config=cfg;
                    item.lua=mods::lua_mod::load(info);item.last_config=std::move(cfg);live.emplace(id,std::move(item));live_order.push_back(id);
                    fprintf(stderr,"[mod-manager] loaded %s (%s)\n",id.c_str(),record.manifest.kind.c_str());
                }catch(const std::exception& e){if(item.lua)mods::lua_mod::unload(*item.lua);record.error=e.what();profile()["enabled"][id]=false;try{save();}catch(...){}}}
            running=!live.empty();
        } catch(const std::exception& e) {last_problem=e.what();}
        dirty=true;
    }catch(const std::exception& e){last_problem=e.what();ready=false;records.clear();fprintf(stderr,"[mod-manager] %s\n",e.what());}
}
std::string directory(){std::lock_guard guard(mutex);return ready?(root/"Mods").string():"";}
std::vector<View> list(){std::lock_guard guard(mutex);std::vector<View> out;for(const auto& [id,r]:records){const auto& m=r.manifest;View v;v.id=id;v.name=m.name;v.version=m.version;v.author=m.author;v.description=m.description;v.kind=m.kind;v.restart_required=content_lifecycle(m);v.enabled=wanted(id);v.active=r.active;v.compatible=m.problem.empty();v.reason=m.problem.empty()?r.error:m.problem;v.status=r.status;if(m.graphics){auto diagnostics=cemu::runtime_status(id);if(!diagnostics.empty())v.status+=". "+diagnostics;}v.options=m.options;auto cfg=config(m);v.pending_restart=v.restart_required&&(v.enabled!=v.active||(m.graphics&&v.active&&!(r.startup_config==cfg)));if(m.graphics&&!m.graphics->shaders.empty()&&!cemu::vulkan()){v.active=false;v.compatible=false;v.reason="GLSL shader packs require Vulkan; choose it in Graphics and restart";}for(auto& o:v.options)o.value=cfg.get(o.id);for(const auto& dep:m.dependencies)v.dependencies.push_back(dep.id+">="+dep.version);v.conflicts=m.conflicts;out.push_back(std::move(v));}return out;}
static bool content_name(std::string n){for(char& c:n)if(c>='A'&&c<='Z')c+='a'-'A';return n=="content";}
bool install(const std::string& source,std::string& error,std::string* installed_id_out){return operation(error,[&]{
    auto nonce=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());auto stage=root/(".stage-"+nonce),backup=root/(".backup-"+nonce);fs::path target;bool backed=false,moved=false;
    try{if(fs::is_regular_file(source)&&content::known_pack(fs::path(source).filename().string())){
        require(!fs::is_symlink(source)&&fs::file_size(source)<=128ull*1024*1024,"Invalid or oversized replacement pack");
        std::ifstream input(source,std::ios::binary);char magic[4]{};input.read(magic,4);require(std::string(magic,4)=="SARC"||std::string(magic,4)=="Yaz0","Replacement pack is not a SARC/Yaz0 archive");
        fs::create_directories(stage);fs::copy_file(source,stage/fs::path(source).filename());
    }else archive::stage(fs::path(source),stage);if(!fs::exists(stage/"manifest.json")){
        bool graphics=false;for(const auto& file:fs::recursive_directory_iterator(stage))if(file.is_regular_file()){
            auto name=file.path().filename().string();if(name.ends_with("_vs.txt")||name.ends_with("_ps.txt"))graphics=true;
            if(name=="rules.txt"){auto text=read_text(file.path());for(char& c:text)if(c>='A'&&c<='Z')c+='a'-'A';if(text.find("[preset]")!=std::string::npos||text.find("[textureredefine]")!=std::string::npos||text.find("[default]")!=std::string::npos)graphics=true;}
        }

        auto named=fs::path(source).lexically_normal();if(named.filename().empty())named=named.parent_path();
        if(content_name(named.filename().string())&&!named.parent_path().filename().empty())named=named.parent_path();
        if(graphics)cemu::import_legacy(stage,named.filename().string());else content::import_legacy(stage,named.filename().string());
    }auto m=manifest(stage);require(m.problem.empty()||m.problem.starts_with("GLSL shader packs require Vulkan"),m.problem);target=root/"Mods"/m.id;auto it=records.find(m.id);require(it==records.end()||(!wanted(m.id)&&!it->second.active&&!it->second.loading),it!=records.end()&&content_lifecycle(it->second.manifest)?"Disable this content mod and restart before updating":"Disable this mod and wait for it to unload before updating");fs::create_directories(target.parent_path());if(fs::exists(target)){fs::rename(target,backup);backed=true;}fs::rename(stage,target);moved=true;m=manifest(target);Record r;r.path=target;r.manifest=std::move(m);auto installed_id=r.manifest.id;records[installed_id]=std::move(r);if(installed_id_out)*installed_id_out=installed_id;if(backed){std::error_code cleanup;fs::remove_all(backup,cleanup);}dirty=true;
    }catch(...){if(moved)fs::remove_all(target);if(backed)fs::rename(backup,target);if(fs::exists(stage))fs::remove_all(stage);throw;}
});}
bool remove(const std::string& id,std::string& error){return operation(error,[&]{auto it=records.find(id);require(it!=records.end(),"Mod not found");require(!wanted(id)&&!it->second.active&&!it->second.loading,content_lifecycle(it->second.manifest)?"Disable this content mod and restart before removing":"Disable this mod and wait for it to unload before removing");for(const auto& [other,r]:records)if(wanted(other))for(const auto& d:r.manifest.dependencies)require(d.id!=id,r.manifest.name+" depends on this mod");auto previous=database;for(auto& [name,p]:database["profiles"].object){p["enabled"].object.erase(id);p["config"].object.erase(id);}try{save();}catch(...){database=previous;throw;}fs::remove_all(it->second.path);records.erase(it);dirty=true;});}
bool enable(const std::string& id,bool on,std::string& error){return operation(error,[&]{require(records.contains(id),"Mod not found");auto enabled=enabled_set();if(on){
    std::set<std::string> visiting;
    std::function<void(const std::string&)> add=[&](const std::string& current){require(!visiting.contains(current),"Dependency cycle at "+current);require(records.contains(current),"Missing dependency: "+current);const auto& graphics=records.at(current).manifest.graphics;if(graphics&&!graphics->shaders.empty())require(cemu::vulkan(),"GLSL shader packs require Vulkan; choose it in Graphics and restart");if(enabled.contains(current))return;visiting.insert(current);for(const auto& d:records.at(current).manifest.dependencies)add(d.id);visiting.erase(current);enabled.insert(current);};add(id);
    }else enabled.erase(id);order(enabled);validate_conflicts(enabled);auto previous=database;for(const auto& [key,r]:records)profile()["enabled"][key]=enabled.contains(key);try{save();}catch(...){database=previous;throw;}for(const auto& key:enabled)records.at(key).error.clear();dirty=true;});}
bool configure(const std::string& id,const std::string& option_id,const Value& value,std::string& error){return operation(error,[&]{require(records.contains(id),"Mod not found");auto& opts=records.at(id).manifest.options;auto it=std::find_if(opts.begin(),opts.end(),[&](const Option& o){return o.id==option_id;});require(it!=opts.end()&&valid_option(*it,value),"Invalid configuration value");auto previous=database;profile()["config"][id][option_id]=value;try{if(records.at(id).manifest.graphics)cemu::validate({{id,*records.at(id).manifest.graphics,config(records.at(id).manifest)}});if(wanted(id))validate_conflicts(enabled_set());save();}catch(...){database=previous;throw;}dirty=true;});}
void disable_all(){std::string ignored;operation(ignored,[&]{auto previous=database;for(const auto& [id,r]:records)profile()["enabled"][id]=false;try{save();}catch(...){database=previous;throw;}dirty=true;});}
std::vector<std::string> profiles(){std::lock_guard guard(mutex);std::vector<std::string> result;if(ready)for(const auto& [name,p]:database.get("profiles").object)result.push_back(name);return result;}
std::string current_profile(){std::lock_guard guard(mutex);return ready?database.get("active").string():"Default";}
bool create_profile(const std::string& name,std::string& error){return operation(error,[&]{require(!name.empty()&&name.size()<=64&&name.find('\0')==std::string::npos,"Invalid profile name");require(database.get("profiles").object.size()<64,"Profile limit reached");require(!database.get("profiles").object.contains(name),"Profile already exists");auto previous=database;Value copy=profile();database["profiles"][name]=std::move(copy);try{save();}catch(...){database=previous;throw;}});}
bool select_profile(const std::string& name,std::string& error){return operation(error,[&]{require(database.get("profiles").object.contains(name),"Profile not found");auto previous=database;database["active"]=name;try{order(enabled_set());validate_conflicts(enabled_set());save();}catch(...){database=previous;throw;}profile_changed=true;dirty=true;});}
bool delete_profile(const std::string& name,std::string& error){return operation(error,[&]{require(name!=database.get("active").string(),"Switch profiles before deleting the active one");require(database.get("profiles").object.contains(name),"Profile not found");auto previous=database;database["profiles"].object.erase(name);try{save();}catch(...){database=previous;throw;}});}
bool refresh(std::string& error){return operation(error,[&]{for(const auto& [id,r]:records)require(!r.active&&!r.loading&&!wanted(id),content_lifecycle(r.manifest)?"Disable content mods and restart before rescanning":"Disable installed mods before rescanning");scan();dirty=true;});}
void frame(uint64_t step){
    if(!dirty.load(std::memory_order_relaxed)&&!running.load(std::memory_order_relaxed))return;
    static uint64_t previous=~uint64_t(0);if(step==previous)return;previous=step;
    if(dirty.exchange(false)){
        std::map<std::string,std::pair<Record,Value>> desired;std::vector<std::string> sequence;bool switching=false;
        {std::lock_guard guard(mutex);switching=profile_changed.exchange(false);try{auto enabled=enabled_set();sequence=order(enabled);validate_conflicts(enabled);std::erase_if(sequence,[](const auto& id){return startup_only(records.at(id).manifest);});for(const auto& id:sequence){desired[id]={records.at(id),config(records.at(id).manifest)};records.at(id).loading=true;}}catch(const std::exception& e){last_problem=e.what();for(auto& [id,r]:records)if(wanted(id))r.error=e.what();sequence.clear();desired.clear();}}
        std::vector<std::string> preserved;
        for(auto it=live_order.rbegin();it!=live_order.rend();++it){
            bool file_backed=false;{std::lock_guard guard(mutex);if(records.contains(*it))file_backed=has_content_files(records.at(*it).manifest);}
            if(file_backed){if(live.contains(*it))preserved.push_back(*it);continue;}
            if(!desired.contains(*it)||switching){auto found=live.find(*it);if(found!=live.end()){if(found->second.lua)mods::lua_mod::unload(*found->second.lua);live.erase(found);}std::lock_guard guard(mutex);if(records.contains(*it)){records.at(*it).active=false;records.at(*it).status.clear();}}
        }
        live_order.assign(preserved.rbegin(),preserved.rend());
        for(const auto& id:sequence){const auto& [record,cfg]=desired.at(id);bool deps_ok=true;for(const auto& dep:record.manifest.dependencies)if(!live.contains(dep.id)){std::lock_guard guard(mutex);if(!records.contains(dep.id)||!records.at(dep.id).active)deps_ok=false;}
            if(!deps_ok){std::lock_guard guard(mutex);records.at(id).loading=false;records.at(id).error="A dependency is unavailable (content dependencies may require restart)";continue;}
            auto it=live.find(id);
            if(it==live.end()){Live item;item.kind=record.manifest.kind;try{mods::lua_mod::LoadInfo info;info.id=record.manifest.id;info.version=record.manifest.version;info.path=record.path.string();info.entry=record.manifest.entry;info.config=cfg;item.lua=mods::lua_mod::load(info);item.last_config=cfg;live.emplace(id,std::move(item));fprintf(stderr,"[mod-manager] loaded %s (%s)\n",id.c_str(),record.manifest.kind.c_str());}catch(const std::exception& e){if(item.lua)mods::lua_mod::unload(*item.lua);std::lock_guard guard(mutex);records.at(id).loading=false;records.at(id).error=e.what();profile()["enabled"][id]=false;try{save();}catch(...){}continue;}}
            live_order.push_back(id);std::lock_guard guard(mutex);records.at(id).active=true;records.at(id).loading=false;records.at(id).error.clear();
        }
        running=!live.empty();
    }
    for(size_t i=0;i<live_order.size();){
        auto id=live_order[i];auto& item=live.at(id);
        Value cfg;{std::lock_guard guard(mutex);cfg=records.contains(id)?config(records.at(id).manifest):Value{};}
        if(item.lua&&!(item.last_config==cfg)){mods::lua_mod::set_config(*item.lua,cfg);mods::lua_mod::config_changed(*item.lua);item.last_config=std::move(cfg);}
        if(item.lua)mods::lua_mod::logic_step(*item.lua,step);
        if(!item.lua||!mods::lua_mod::faulted(*item.lua)){
            if(item.lua){auto st=mods::lua_mod::status(*item.lua);std::lock_guard guard(mutex);if(records.contains(id))records.at(id).status=std::move(st);}
            ++i;continue;
        }
        auto msg=mods::lua_mod::fault_message(*item.lua);mods::lua_mod::unload(*item.lua);live.erase(id);live_order.erase(live_order.begin()+static_cast<std::ptrdiff_t>(i));
        {std::lock_guard guard(mutex);if(records.contains(id)){records.at(id).active=false;records.at(id).status.clear();records.at(id).error=msg;profile()["enabled"][id]=false;try{save();}catch(...){}}}
        running=!live.empty();
    }
}
}
