#include "manager.h"

namespace mods::manager {

std::span<const Entry> entries() { return {}; }
const Entry* find(std::string_view) { return nullptr; }
void load_saved() {}
bool set_enabled(std::string_view, bool) { return false; }
void disable_all() {}

}
