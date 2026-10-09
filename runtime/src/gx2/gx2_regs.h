

#pragma once
#include "Cafe/HW/Latte/ISA/LatteReg.h"
#include "Cafe/HW/Latte/ISA/RegDefines.h"
#include "Cafe/HW/Latte/Core/LatteConst.h"

namespace gx2 {

constexpr uint32 kNumRegs = 0x10000;

void bind_vertex_shader_regs(uint32* regs, uint32 shader);
void bind_pixel_shader_regs(uint32* regs, uint32 shader);
void bind_geometry_shader_regs(uint32* regs, uint32 shader);
uint32 vertex_shader_program(uint32 shader, uint32* size);
uint32 pixel_shader_program(uint32 shader, uint32* size);

}
