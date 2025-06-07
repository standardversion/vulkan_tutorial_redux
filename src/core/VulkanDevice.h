#pragma once
#include "vulkan/vulkan.h"
#include <vector>
#include <optional>
#include <string>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphics;
	std::optional<uint32_t> present;
};

class VulkanDevice
{
public:
	VulkanDevice() = default;
	VulkanDevice(const VulkanDevice&) = delete;
	VulkanDevice& operator=(const VulkanDevice&) = delete;
	void pick_physical_device(VkInstance instance, VkSurfaceKHR surface);
	void create_logical_device(VkInstance instance);
	void cleanup();
private:
	VkPhysicalDevice m_physical_device{ VK_NULL_HANDLE };
	VkDevice m_device{ VK_NULL_HANDLE };
	VkPhysicalDeviceFeatures m_features;
	QueueFamilyIndices m_queue_families;
	VkQueue m_graphics_queue{ VK_NULL_HANDLE };
	VkQueue m_present_queue{ VK_NULL_HANDLE };
	const std::vector<const char*> m_requested_extensions{ { "VK_KHR_swapchain" } };
	const std::vector<VkFormat> m_requested_surface_formats{ { VK_FORMAT_R8G8B8A8_SRGB } };
	const std::vector<VkPresentModeKHR> m_requested_present_modes{ { VK_PRESENT_MODE_FIFO_KHR } };

	bool is_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface) noexcept;
	bool check_features_support(VkPhysicalDevice physical_device) noexcept;
	bool check_extensions_support(VkPhysicalDevice physical_device) const noexcept;
	bool check_formats_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface) const noexcept;
	bool check_present_modes_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface) const noexcept;
	VkDeviceQueueCreateInfo create_queue_create_info(uint32_t queue_family_index, const float* priority) const noexcept;
};