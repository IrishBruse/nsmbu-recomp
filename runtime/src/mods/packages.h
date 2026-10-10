#pragma once
#include "mod_json.h"
#include <cstdint>
#include <string>
#include <vector>
namespace mods::packages {
inline constexpr const char* kGameId="nsmbu-usa";
inline constexpr const char* kManagerVersion="1.3.0";
struct Option {
    std::string id,name,description,type;
    json::Value value,default_value;
    double minimum=0,maximum=1,step=1;
    std::vector<std::string> choices;
};
struct View {
    std::string id,name,version,author,description,kind,reason,status;
    bool enabled=false,active=false,compatible=false,restart_required=false,pending_restart=false;
    std::vector<Option> options;
    std::vector<std::string> dependencies,conflicts;
};

void initialize();
std::string directory();
std::vector<View> list();
bool install(const std::string& source,std::string& error,std::string* installed_id=nullptr);
bool remove(const std::string& id,std::string& error);
bool enable(const std::string& id,bool on,std::string& error);

bool configure(const std::string& id,const std::string& option,const json::Value& value,std::string& error);
void disable_all();
std::vector<std::string> profiles();
std::string current_profile();
bool create_profile(const std::string& name,std::string& error);
bool select_profile(const std::string& name,std::string& error);
bool delete_profile(const std::string& name,std::string& error);

void frame(uint64_t step);
bool refresh(std::string& error);
}
