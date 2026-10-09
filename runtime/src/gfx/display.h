

#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace gfx {

struct Box { float x = 0, y = 0, w = 0, h = 0; };

struct PresentPlan {
    float dw = 0, dh = 0;
    bool sim = false;
    Box tv, pip;
    bool pip_on = false;
    bool pip_wanted = false;
    float scale = 1;
    float pip_opacity = 1;
    bool drc_window = false;
    int filter = 0;
    bool sample_auto = false;
    bool drc_only = false;
    Box button;
    int button_dot = 0;
};
constexpr uint32_t kSignatureW = 32, kSignatureH = 18;

PresentPlan display_plan(bool haveTv, float tvW, float tvH, bool haveDrc, float drcW, float drcH, float layerW, float layerH,
                         uint64_t frame);

PresentPlan display_plan_for(const PresentPlan& p, float dw, float dh, float tvW, float tvH, float drcW, float drcH);

Box display_layout(float dw, float dh, float tw, float th);

void display_auto_signature(const std::vector<float>& drc, const std::vector<float>* tv, uint64_t frame);

std::vector<std::string> display_take_present_dumps();
void request_present_dump(const std::string& path);
void display_log_present_dump(const std::string& path, const PresentPlan& p, float tvW, float tvH);

}
