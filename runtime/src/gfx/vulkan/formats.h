

#pragma once
#include <vulkan/vulkan.h>

#include <cstdint>

namespace gfxvk {

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
    VkFormat pixel = VK_FORMAT_UNDEFINED;
    uint32_t bytesPerBlock = 0;
    uint32_t hostBytesPerBlock = 0;
    bool compressed = false;
    bool depth = false;
    bool stencil = false;
    Convert convert = Convert::NONE;
    enum Kind : uint8_t { FLOAT, UINT, SINT } kind = FLOAT;
};

FormatInfo format_info(uint32_t gx2Format, bool isDepth);
FormatInfo cpu_copy_format(uint32_t gx2Format);

void convert_row(Convert c, const uint8_t* src, uint8_t* dst, uint32_t count);

}
