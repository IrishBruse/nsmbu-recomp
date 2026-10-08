#include "mods.h"

namespace mods {

bool mods_enabled() {
#ifdef NSMBU_MODS_ENABLED
    return true;
#else
    return false;
#endif
}

}
