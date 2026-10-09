#pragma once

#include <bit>
#include <cstdint>
#include <cstring>
#include <string>

namespace gfx::area_sample {

inline uint64_t program_hash(const void* bytes, uint32_t size) {
    uint64_t a = 0, b = 0;
    for (uint32_t i = 0; i < size / 4; i++) {
        uint32_t word;
        std::memcpy(&word, static_cast<const uint8_t*>(bytes) + i * 4, 4);
        a = std::rotl(a + word, 3);
        b = std::rotr(b ^ word, 7);
    }
    return a + b;
}

inline uint32_t units_for_pixel_shader(const void* bytes, uint32_t size) {
    if (!bytes || size != 448) return 0;
    switch (program_hash(bytes, size)) {
    case 0x51a7ccdf69184627ull:
    case 0x16285301a96cbf8dull:
        return 1u;
    default:
        return 0;
    }
}

inline int rewrite(std::string& src, uint32_t units, bool msl) {
    const size_t entry = src.find(msl ? "fragment FragmentOut main0" : "void main(");
    if (!units || entry == std::string::npos) return 0;
    int count = 0;
    for (uint32_t unit = 0; unit < 32; unit++) {
        if (!(units >> unit & 1)) continue;
        const std::string n = std::to_string(unit);
        const std::string from = msl ? "tex" + n + ".sample(samplr" + n + ", " : "texture(textureUnitPS" + n + ", ";
        const std::string to = msl ? "nsmbu_area_sample(tex" + n + ", samplr" + n + ", supportBuffer.tex" + n + "Scale, "
                                   : "nsmbuAreaSample(textureUnitPS" + n + ", uf_tex" + n + "Scale, ";
        for (size_t at = src.find(from); at != std::string::npos; at = src.find(from, at + to.size())) {
            src.replace(at, from.size(), to);
            count++;
        }
    }
    if (!count) return 0;
    static const char* const kMsl =
        "// area-sampled tap for upscaled render targets (runtime/src/gfx/area_sample.h)\n"
        "static float4 nsmbu_area_sample(texture2d<float> t, sampler s, float2 scale, float2 uv) {\n"
        "    float2 k = ceil(scale - 0.001);\n"
        "    if (k.x <= 1.0 && k.y <= 1.0) return t.sample(s, uv);\n"
        "    float2 step = scale / (float2(t.get_width(), t.get_height()) * k);\n"
        "    float4 sum = float4(0.0);\n"
        "    for (float j = 0.5; j < k.y; j += 1.0)\n"
        "        for (float i = 0.5; i < k.x; i += 1.0) sum += t.sample(s, uv + (float2(i, j) - 0.5 * k) * step);\n"
        "    return sum / (k.x * k.y);\n"
        "}\n";
    static const char* const kGlsl =
        "// area-sampled tap for upscaled render targets (runtime/src/gfx/area_sample.h)\n"
        "vec4 nsmbuAreaSample(sampler2D t, vec2 scale, vec2 uv) {\n"
        "    vec2 k = ceil(scale - 0.001);\n"
        "    if (k.x <= 1.0 && k.y <= 1.0) return texture(t, uv);\n"
        "    vec2 step = scale / (vec2(textureSize(t, 0)) * k);\n"
        "    vec4 sum = vec4(0.0);\n"
        "    for (float j = 0.5; j < k.y; j += 1.0)\n"
        "        for (float i = 0.5; i < k.x; i += 1.0) sum += texture(t, uv + (vec2(i, j) - 0.5 * k) * step);\n"
        "    return sum / (k.x * k.y);\n"
        "}\n";
    src.insert(src.find(msl ? "fragment FragmentOut main0" : "void main("), msl ? kMsl : kGlsl);
    return count;
}

}
