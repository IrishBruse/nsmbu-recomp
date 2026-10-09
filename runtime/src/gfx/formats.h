

#pragma once
#include <Metal/Metal.h>

#include <cstdint>

namespace gfx {

enum class Convert : uint8_t {
    NONE,
    RGB565,
    RGBA5551,
    ABGR1555,
    RGBA4,
    RG4,
    D24S8,
    D24_R32F,
    X24_8_32F,
};

struct FormatInfo {
    MTLPixelFormat pixel = MTLPixelFormatInvalid;
    uint32_t bytesPerBlock = 0;
    uint32_t hostBytesPerBlock = 0;
    bool compressed = false;
    bool depth = false;
    bool stencil = false;
    Convert convert = Convert::NONE;
    enum Kind : uint8_t { FLOAT, UINT, SINT } kind = FLOAT;
};

FormatInfo format_info(uint32_t gx2Format, bool isDepth);

void convert_row(Convert c, const uint8_t* src, uint8_t* dst, uint32_t count);

}
