#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "gfx/display.h"
struct ImDrawData;
namespace gfxvk {
struct Screen;
struct Surface;
struct Buffer;

struct ComposeQuad {
 Surface* image=nullptr; bool sourceLinear=false; gfx::Box box; float alpha=1;
 bool solid=false; float color[4]{};
};

void set_present_plan(const gfx::PresentPlan* plan);
std::vector<ComposeQuad> screen_quads(Screen& screen,VkExtent2D target,int& filter);

std::vector<uint8_t> compose_offscreen(Screen& screen,uint32_t width,uint32_t height,bool srgb);

bool record_screenshot(Screen& screen,Buffer& buffer,uint32_t& width,uint32_t& height,bool& bgra);

bool record_signature(int slot,Surface& source,bool sourceLinear);
std::vector<float> read_signature(int slot);
void reset_signatures();

std::vector<uint8_t> read_surface_rgba(Surface& source,bool encodeSrgb);

void reset_present_screen(Screen& screen);
void prepare_present_screen(Screen& screen, bool colorAttachmentSupported, bool captureTransferSupported = false);

bool draw_present_screen(Screen& screen, uint32_t imageIndex);

bool present_capture_requested();
void record_present_capture(Screen& screen, uint32_t imageIndex);
void finish_present_capture(Screen& screen);
void write_rgba_png(const std::string& path, uint32_t width, uint32_t height,
                    const std::vector<uint8_t>& rgba);

void set_overlay_draw(ImDrawData* draw);
void overlay_renderer_init();
void overlay_prepare(ImDrawData* draw);
void overlay_draw(ImDrawData* draw,VkCommandBuffer cmd,VkFormat format,VkExtent2D extent,bool linear);
void reset_overlay_resources();

void reset_present_resources();
}
