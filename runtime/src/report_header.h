

#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace reporthdr {

constexpr int kVulkanCpuPathCount = 15;
extern const char* const kVulkanCpuPaths[kVulkanCpuPathCount];

std::string vk_version(uint32_t v);

std::string driver_version(uint32_t vendorID, uint32_t driverVersion, bool windows);

std::string vulkan_gpu(const char* deviceName, uint32_t vendorID, uint32_t driverVersion, uint32_t apiVersion,
                       bool windows);

std::string os_description();

std::string windows_name(uint32_t major, uint32_t minor, uint32_t build, const std::string& displayVersion,
                         uint32_t ubr);
std::string os_release_pretty_name(const std::string& osRelease);

using GetEnv = const char* (*)(const char* name);
std::vector<std::string> vulkan_overrides(GetEnv env);

struct Info {
    std::string version, commit;
    std::string os;
    std::string gpu;
    std::string renderer, host, fps;
    float scale = 1;
    int bufferCache = -1;
    std::string gyro;
    std::vector<std::string> overrides;
};

std::string format(const Info& info);

}
