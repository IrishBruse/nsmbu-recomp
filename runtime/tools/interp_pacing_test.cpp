

#include "../src/interp_pacing.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>

using namespace interp::pacing;

static int g_failed = 0;
#define CHECK(cond)                                                                \
    do {                                                                           \
        if (!(cond)) {                                                             \
            std::fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
            g_failed++;                                                            \
        }                                                                          \
    } while (0)
#define CHECK_NEAR(a, b, eps) CHECK(std::fabs(double(a) - double(b)) <= (eps))

static void test_fractions() {

    CHECK(pass_fraction(0, 1, false) == 0.5f);
    CHECK(pass_fraction(1, 1, false) == 1.0f);

    for (int k = 0; k <= 3; k++) CHECK(pass_fraction(k, 3, false) == float(k + 1) / 4.0f);

    for (int k = 0; k <= 7; k++) CHECK(pass_fraction(k, 7, false) == float(k + 1) / 8.0f);

    CHECK(pass_fraction(0, 1, true) == 1.0f);
    CHECK(pass_fraction(1, 1, true) == 1.0f);

    CHECK_NEAR(pass_fraction(0, 2, false), 1.0 / 3.0, 1e-7);
    CHECK_NEAR(pass_fraction(1, 2, false), 2.0 / 3.0, 1e-7);
    CHECK(pass_fraction(2, 2, false) == 1.0f);
}

static void test_blends() {
    std::mt19937 rng(7);
    std::uniform_real_distribution<float> u(-1000.0f, 1000.0f);
    for (int i = 0; i < 10000; i++) {
        const float a = u(rng), b = u(rng);

        const float h = lerp_f(a, b, 0.5f), o = 0.5f * (a + b);
        CHECK(std::memcmp(&h, &o, 4) == 0);

        for (int k = 0; k <= 8; k++) {
            const float t = k / 8.0f;
            if (t == 0.5f) continue;
            CHECK_NEAR(lerp_f(a, b, t), a + (b - a) * double(t), 1e-3);
        }
        CHECK(lerp_f(a, b, 1.0f) == b);
        CHECK(lerp_f(a, b, 0.0f) == a);
    }

    for (int a = -32768; a < 32768; a += 997)
        for (int b = -32768; b < 32768; b += 1009) {
            const int16_t d = (int16_t)(b - a);
            CHECK(lerp_s16((int16_t)a, (int16_t)b, 0.5f) == (int16_t)(a + d / 2));
            CHECK(lerp_s16((int16_t)a, (int16_t)b, 1.0f) == (int16_t)b);
        }
    CHECK(lerp_s16(32000, -32000, 0.25f) == (int16_t)(32000 + 384));
    CHECK(lerp_s16(-32000, 32000, 0.75f) == (int16_t)(-32000 - 1152));

    CHECK(lerp_u8(0, 255, 0.5f) == 128);
    CHECK(lerp_u8(0, 255, 0.25f) == 64);
    CHECK(lerp_u8(200, 100, 0.125f) == 188);
    CHECK(lerp_u8(10, 20, 1.0f) == 20);
}

static void test_slerp() {
    float wa, wb;
    slerp_weights(0.3f, 0.5f, wa, wb);
    CHECK(wa == 1.0f && wb == 1.0f);
    for (double ang : {0.01, 0.3, 0.785, 1.5}) {
        const float a[2] = {1, 0}, b[2] = {(float)std::cos(ang), (float)std::sin(ang)};
        for (int k = 1; k < 8; k++) {
            const float t = k / 8.0f;
            slerp_weights(a[0] * b[0] + a[1] * b[1], t, wa, wb);
            const float x = wa * a[0] + wb * b[0], y = wa * a[1] + wb * b[1];
            CHECK_NEAR(std::atan2(y, x), t * ang, 1e-4);
            if (t != 0.5f) CHECK_NEAR(std::hypot(x, y), 1.0, 1e-4);
        }
    }
    slerp_weights(1.0f, 0.25f, wa, wb);
    CHECK_NEAR(wa, 0.75, 1e-6);
    CHECK_NEAR(wb, 0.25, 1e-6);
}

static void test_cap() {
    CHECK(cap_fps(240, 120, true) == 120);
    CHECK(cap_fps(240, 60, true) == 60);
    CHECK(cap_fps(120, 60, true) == 60);
    CHECK(cap_fps(120, 144, true) == 120);
    CHECK(cap_fps(240, 144, true) == 120);
    CHECK(cap_fps(240, 165, true) == 165);
    CHECK(cap_fps(165, 165, true) == 165);
    CHECK(cap_fps(165, 144, true) == 120);
    CHECK(cap_fps(240, 240, true) == 240);
    CHECK(cap_fps(240, 239, true) == 240);
    CHECK(cap_fps(120, 119, true) == 120);
    CHECK(cap_fps(120, 90, true) == 60);
    CHECK(cap_fps(60, 30, true) == 60);
    CHECK(cap_fps(240, 0, true) == 240);
    CHECK(cap_fps(240, 60, false) == 240);
    CHECK(cap_fps(100, 240, true) == 60);
}

static void test_plan_and_next() {
    const int64_t budget = 35'333'333, vs = 16'683'333;
    CHECK(plan_in_between(3, budget, vs / 2, vs / 2) == 3);
    CHECK(plan_in_between(7, budget, vs / 4, vs / 4) == 7);
    CHECK(plan_in_between(7, budget, vs / 2, vs / 2) == 3);
    CHECK(plan_in_between(7, budget, vs, vs) == 1);
    CHECK(plan_in_between(3, budget, 25'000'000, vs / 2) == 1);
    CHECK(plan_in_between(7, budget, 12'000'000, vs / 4) == 5);
    CHECK(plan_in_between(3, budget, 200'000'000, vs) == 1);
    CHECK(plan_in_between(1, budget, 1, 1) == 1);
    CHECK(plan_in_between(0, budget, 0, 0) == 1);

    CHECK(next_pass(0, 1, 16'000'000, 16'000'000, budget) == Next::kHold);
    CHECK(next_pass(0, 1, 20'000'000, 16'000'000, budget) == Next::kDrop);
    CHECK(next_pass(1, 1, 0, 0, budget) == Next::kStepDone);

    CHECK(next_pass(0, 3, 8'000'000, 8'000'000, budget) == Next::kHold);
    CHECK(next_pass(1, 3, 20'000'000, 8'000'000, budget) == Next::kRecord);
    CHECK(next_pass(1, 3, 30'000'000, 8'000'000, budget) == Next::kDrop);
    CHECK(next_pass(2, 3, 26'000'000, 8'000'000, budget) == Next::kHold);
    CHECK(next_pass(3, 3, 99'000'000, 8'000'000, budget) == Next::kStepDone);
}

struct SimResult {
    double steps_per_s, frames_per_s, holds_per_step, exact_share, record_share;
};
static SimResult simulate(int fps, double logic_ms, double hold_ms, double seconds = 30, bool paced = true, double spike_at = -1,
                          double spike_ms = 0) {
    const double kStep = 33.333333, kBudget = kStep + 2.0, period = 16.683333 * 60.0 / fps, kProbeAfter = 10000;
    const int n_max = fps / 30 - 1;
    double t = 0, last_logic = -1e9, logic_avg = period, hold_avg = period, last_hold = 0;
    bool probe = false;
    bool wait_step = false, prev_record = false;
    long steps = 0, frames = 0, holds = 0, exact = 0, records = 0;
    auto run_pass = [&](double cost, double& avg) {
        const double start = t;
        if (spike_at >= 0 && t >= spike_at) {
            cost += spike_ms;
            spike_at = -1;
        }
        t = std::ceil((t + cost) / period - 1e-9) * period;
        avg = probe ? t - start : update_average(int64_t(avg * 1e6), int64_t((t - start) * 1e6)) / 1e6;
        probe = false;
        frames++;
    };
    while (t < seconds * 1000) {

        if (paced && wait_step && t < last_logic + kStep) t = last_logic + kStep;
        wait_step = false;
        last_logic = t;
        const bool is_exact = paced && !prev_record;
        exact += is_exact;
        int n = paced ? plan_in_between(n_max, int64_t(kBudget * 1e6), int64_t(logic_avg * 1e6), int64_t(hold_avg * 1e6)) : n_max;
        if (is_exact) n = 1;
        steps++;
        run_pass(logic_ms, logic_avg);
        prev_record = false;
        for (int phase = 0;;) {

            const bool try_probe = paced && phase == 0 && t - last_hold >= kProbeAfter;
            const double hold_est = try_probe ? period : hold_avg;
            Next next = paced ? next_pass(phase, n, int64_t((t - last_logic) * 1e6), int64_t(hold_est * 1e6), int64_t(kBudget * 1e6))
                              : (phase >= n ? Next::kStepDone : Next::kHold);
            if (next == Next::kStepDone) {
                prev_record = true;
                records++;
                if (n_max > 1) wait_step = true;
                break;
            }
            if (next == Next::kDrop) {
                wait_step = true;
                break;
            }
            phase = next == Next::kRecord ? n : phase + 1;
            holds++;
            probe = try_probe;
            last_hold = t;
            run_pass(hold_ms, hold_avg);
        }
    }
    const double s = t / 1000;
    return {steps / s, frames / s, double(holds) / steps, double(exact) / steps, double(records) / steps};
}

static void test_simulation() {

    SimResult r = simulate(120, 3, 2);
    std::printf("120 fps, cheap: %.2f steps/s, %.1f fps, %.2f in-between per step\n", r.steps_per_s, r.frames_per_s, r.holds_per_step);
    CHECK_NEAR(r.steps_per_s, 29.97, 0.05);
    CHECK_NEAR(r.holds_per_step, 3.0, 0.03);
    CHECK_NEAR(r.frames_per_s, 119.9, 0.6);
    r = simulate(240, 2, 1.5);
    std::printf("240 fps, cheap: %.2f steps/s, %.1f fps, %.2f in-between per step\n", r.steps_per_s, r.frames_per_s, r.holds_per_step);
    CHECK_NEAR(r.steps_per_s, 29.97, 0.05);
    CHECK_NEAR(r.holds_per_step, 7.0, 0.05);
    r = simulate(60, 5, 3);
    CHECK_NEAR(r.steps_per_s, 29.97, 0.05);
    CHECK_NEAR(r.holds_per_step, 1.0, 0.01);

    r = simulate(240, 6, 6);
    std::printf("240 fps, 6 ms passes: %.2f steps/s, %.1f fps, %.2f in-between per step\n", r.steps_per_s, r.frames_per_s, r.holds_per_step);
    CHECK(r.steps_per_s > 29.5);
    CHECK(r.holds_per_step >= 2.9);
    CHECK(r.record_share > 0.99);

    r = simulate(120, 10, 10);
    std::printf("120 fps, 10 ms passes: %.2f steps/s, %.1f fps, %.2f in-between per step, %.0f%% exact\n", r.steps_per_s, r.frames_per_s,
                r.holds_per_step, 100 * r.exact_share);
    CHECK(r.steps_per_s > 29.5);
    CHECK(r.holds_per_step >= 0.99);
    CHECK(r.exact_share < 0.01);

    r = simulate(240, 12, 3);
    std::printf("240 fps, 12 ms logic, 3 ms holds: %.2f steps/s, %.1f fps, %.2f in-between per step\n", r.steps_per_s, r.frames_per_s,
                r.holds_per_step);
    CHECK(r.steps_per_s > 29.5);
    CHECK(r.holds_per_step >= 4.9);

    r = simulate(120, 24, 4);
    std::printf("120 fps, 24 ms logic: %.2f steps/s, %.1f fps, %.2f in-between per step, %.0f%% exact\n", r.steps_per_s, r.frames_per_s,
                r.holds_per_step, 100 * r.exact_share);
    CHECK(r.steps_per_s > 29.5);
    CHECK(r.exact_share < 0.01);

    r = simulate(120, 30, 30);
    std::printf("120 fps, 30 ms passes: %.2f steps/s, %.1f fps, %.2f in-between per step\n", r.steps_per_s, r.frames_per_s, r.holds_per_step);
    CHECK(r.steps_per_s > 28.5);
    CHECK(r.holds_per_step < 0.1);

    r = simulate(120, 5, 30, 60);
    std::printf("120 fps, 5 ms logic, 30 ms holds: %.2f steps/s, %.1f fps, %.3f in-between per step\n", r.steps_per_s, r.frames_per_s,
                r.holds_per_step);
    CHECK(r.steps_per_s > 29.5);
    CHECK(r.holds_per_step < 0.01);

    r = simulate(120, 3, 2, 30, true, 5000, 70);
    std::printf("120 fps, cheap, one 70 ms hitch: %.2f steps/s, %.1f fps, %.2f in-between per step\n", r.steps_per_s, r.frames_per_s,
                r.holds_per_step);
    CHECK(r.holds_per_step > 2.95);
    CHECK(r.steps_per_s > 29.8);
    CHECK(update_average(8'000'000, 80'000'000) == (3 * 8'000'000 + 16'000'000) / 4);
    CHECK(update_average(8'000'000, 4'000'000) == 7'000'000);

    r = simulate(120, 30, 30, 30, false);
    CHECK(r.steps_per_s < 10);
}

int main() {
    test_fractions();
    test_blends();
    test_slerp();
    test_cap();
    test_plan_and_next();
    test_simulation();
    if (g_failed) {
        std::fprintf(stderr, "interp_pacing_test: %d check(s) failed\n", g_failed);
        return 1;
    }
    std::printf("interp_pacing_test: all checks passed\n");
    return 0;
}
