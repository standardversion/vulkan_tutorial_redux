#pragma once
#include "vulkan/vulkan.h"
#include <string>

struct WindowCfg {
    uint32_t width{ 800 };
    uint32_t height{ 600 };
    std::string title{ "Vulkan" };
};

struct AppInfo {
    uint32_t major{ 1 };
    uint32_t minor{ 0 };
    uint32_t patch{ 0 };
    uint32_t engine_major{ 0 };
    uint32_t engine_minor{ 0 };
    uint32_t engine_patch{ 0 };
    std::string engine_name{ "No engine" };
    std::string name{ "My Vulkan App" };
    uint32_t vulkan_api_version{VK_API_VERSION_1_4};
};

struct CommandPoolCfg {
    uint32_t max_frames_in_flight{ 2 };
    uint32_t buffers_per_frame{ 1 };
};

struct VulkanCfg {
    AppInfo app_info;
    CommandPoolCfg command_pool;
};

struct Config {
    WindowCfg window;
    VulkanCfg vulkan_cfg;
};

