#include "gfx/renderer.h"
#include "ppc.h"

extern "C" void site_022A79C4(Cpu* c) {
    if (!render::g_backend)
        return;
    const float scale = render::res_scale();
    if (!(scale > 0.f) || scale == 1.f)
        return;
    const double k = 1.0 / static_cast<double>(scale);
    c->f[7].ps0 *= k;
    c->f[9].ps0 *= k;
}
