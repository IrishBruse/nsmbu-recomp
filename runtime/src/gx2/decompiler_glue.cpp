
#ifdef NSMBU_HAS_VULKAN
#include "Cafe/HW/Latte/Renderer/Vulkan/VulkanRenderer.h"
#endif
#ifdef NSMBU_HAS_METAL
#include "Cafe/HW/Latte/Renderer/Metal/MetalRenderer.h"
#endif
#include "gfx/renderer.h"
#include "runtime.h"

#ifdef NSMBU_HAS_METAL
std::unique_ptr<Renderer> g_renderer = std::make_unique<MetalRenderer>();
#else
std::unique_ptr<Renderer> g_renderer = std::make_unique<VulkanRenderer>();
#endif
void select_decompiler_api(render::Api api) {
#ifdef NSMBU_HAS_VULKAN
    if (api == render::Api::Vulkan) {
        g_renderer = std::make_unique<VulkanRenderer>();
        return;
    }
#endif
#ifdef NSMBU_HAS_METAL
    g_renderer = std::make_unique<MetalRenderer>();
#endif
}
void cemu_shim_log(const std::string& msg) { LOG("[decompiler] %s", msg.c_str()); }
