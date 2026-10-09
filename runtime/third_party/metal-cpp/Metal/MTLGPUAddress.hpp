

#pragma once

#ifdef __METAL_VERSION__

#include <metal_stdlib>

#else

#include <cstdint>

#endif

namespace MTL
{
    using GPUAddress = uint64_t;
}
