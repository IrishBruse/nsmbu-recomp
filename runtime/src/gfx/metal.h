#include <atomic>

#pragma once
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "formats.h"

namespace gfx {
void peek_z(const uint32_t*, uint32_t);

struct Surface {
    std::shared_ptr<Surface> mipChain;
    uint64_t mipChainSeq = ~0ull;
    id<MTLTexture> tex = nil;
    uint32_t addr = 0, mipAddr = 0;
    uint32_t width = 0, height = 0, slices = 1, pitch = 0, mips = 1;
    uint32_t format = 0;
    uint32_t dim = 1;
    uint32_t tileMode = 0;
    uint32_t swizzle = 0;
    bool isDepth = false;
    bool gpuWritten = false;
    uint64_t writeSeq = 0;
    bool formatViews = false;
    uint64_t writtenBackSeq = 0;
    uint64_t contentHash = 0;
    uint64_t lastCheckedFrame = ~0ull;
    uint64_t sparseHash = 0;
    uint32_t dataSize = 0;
    bool dirty = true;

    std::vector<std::pair<uint32_t, uint32_t>> levelRanges;
    uint64_t watchStamp = 0;
    bool watched = false;
    FormatInfo fmt;

    float scale = 1.0f;
    float ax = 1.0f, ay = 1.0f;
    float sx = 1.0f, sy = 1.0f;
};

struct Screen {
    id<MTLTexture> tex = nil;
    CAMetalLayer* layer = nil;
    std::atomic<bool> srgb{false};
    std::atomic<bool> visible{true};
};

uint64_t next_write_seq();
inline void mark_gpu_written(Surface* s) { s->gpuWritten = true; s->writeSeq = next_write_seq(); }

struct Renderer {
    id<MTLDevice> device = nil;
    id<MTLCommandQueue> queue = nil;

    id<MTLCommandBuffer> cmd = nil;
    id<MTLRenderCommandEncoder> enc = nil;
    bool binding = false;

    Surface* passColor[8] = {};
    uint32_t mainDepthAddr = 0;
    Surface* passDepth = nullptr;
    uint32_t passColorSlice[8] = {}, passDepthSlice = 0;

    struct GuestRange {
        uint32_t base, size;
        id<MTLBuffer> buf;
    };
    std::vector<GuestRange> guest;

    Screen tv, drc;
    id<MTLRenderPipelineState> presentPipeline = nil;
    id<MTLRenderPipelineState> presentPipelineSRGB = nil;
    id<MTLSamplerState> linearClamp = nil;

    std::unordered_multimap<uint32_t, std::unique_ptr<Surface>> surfaces;
    std::vector<Surface*> linearTargets;

    uint64_t frame = 0;
    uint64_t drawCount = 0;
};
extern Renderer R;

id<MTLCommandBuffer> command_buffer();
void end_encoder();

id<MTLBuffer> guest_buffer(uint32_t addr, uint32_t* offset);

struct SurfaceDesc {
    uint32_t addr = 0, mipAddr = 0, width = 0, height = 0, slices = 1, pitch = 0, mips = 1;
    uint32_t format = 0, dim = 1, tileMode = 0, swizzle = 0;
    bool isDepth = false;
};
Surface* find_or_create_surface(const SurfaceDesc& d, bool forRendering);

Surface* color_target(const uint32_t* regs, int index, uint32_t* slice = nullptr);
Surface* depth_target(const uint32_t* regs, uint32_t* slice = nullptr);

Surface* surface_from_color_buffer(uint32_t gx2ColorBuffer, uint32_t* firstSlice = nullptr, uint32_t* numSlices = nullptr);
Surface* surface_from_depth_buffer(uint32_t gx2DepthBuffer, uint32_t* firstSlice = nullptr, uint32_t* numSlices = nullptr);
Surface* sampled_texture(const uint32_t* texWords, bool isDepthSampler);
void upload_surface(Surface* s);

float res_scale();
void set_res_scale(float f);
void latch_res_scale();

void set_frame_aspect(float a);

void resample(id<MTLTexture> src, id<MTLTexture> dst, const FormatInfo& fmt, uint32_t slices, float uMax = 1, float vMax = 1,
              uint32_t dstW = 0, uint32_t dstH = 0);
void forget_texture_views();

}
