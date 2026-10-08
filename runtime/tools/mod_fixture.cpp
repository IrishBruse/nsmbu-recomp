#include "nsmbu_mod.h"
static const NSMBUModHostV1* host;
static void frame(void*,uint64_t) {host->status(host->context,host->get_string(host->context,"label"));}
extern "C" NSMBU_MOD_EXPORT int nsmbu_mod_init_v1(const NSMBUModHostV1* h, NSMBUModV1* api) {
    if(h->abi_version != 1) return 0;
    host=h;api->size=sizeof(*api);api->abi_version=1;api->on_frame=frame;return 1;
}
