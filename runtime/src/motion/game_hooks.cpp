

#include "input.h"
#include "motion.h"
#include "runtime.h"

extern "C" void site_0261864C(Cpu* c) {
    if (c->r[3] != 0 && input::pro_controller() && motion::drives_gamepad()) c->r[3] = 0;
}
