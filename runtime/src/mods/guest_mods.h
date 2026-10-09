
#pragma once
#include <cstdint>
#include "guest_identity.h"
namespace guestmods {
bool hooks_built();
inline constexpr uint32_t kRegionStart=0x7F000000,kRegionSize=0x01000000;
std::vector<ModIdentity> enabled_mods();
void init();
void frame(uint64_t step);
}
