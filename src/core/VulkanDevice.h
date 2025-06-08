#pragma once
#include "vulkan/vulkan.h"
#include <vector>
#include <optional>
#include <string>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphics;
	std::optional<uint32_t> present;

	std::vector<uint32_t> get_indices() const noexcept;
};

class VulkanDevice
{
public:
	VulkanDevice() = default;
	VulkanDevice(const VulkanDevice&) = delete;
	VulkanDevice& operator=(const VulkanDevice&) = delete;
	VkResult pick_physical_device(VkInstance instance, VkSurfaceKHR surface) noexcept;
	VkResult create_logical_device(VkInstance instance) noexcept;
	void cleanup();
	VkPhysicalDevice get_physical_device() const noexcept;
	VkDevice get_logical_device() const noexcept;
	QueueFamilyIndices get_queue_family_indices() const noexcept;
	VkSurfaceFormatKHR  pick_format(const std::vector<VkSurfaceFormatKHR>& preferred_formats) const noexcept;
	VkSurfaceFormatKHR  pick_format() const noexcept;
	VkPresentModeKHR pick_present_mode(const std::vector<VkPresentModeKHR>& preferred_modes) const noexcept;
	VkPresentModeKHR pick_present_mode() const noexcept;
private:
	VkPhysicalDevice m_physical_device{ VK_NULL_HANDLE };
	VkDevice m_device{ VK_NULL_HANDLE };
	VkPhysicalDeviceFeatures m_features;
	QueueFamilyIndices m_queue_families;
	VkQueue m_graphics_queue{ VK_NULL_HANDLE };
	VkQueue m_present_queue{ VK_NULL_HANDLE };
	const std::vector<const char*> m_requested_extensions{ { "VK_KHR_swapchain" } };
	const std::vector<VkSurfaceFormatKHR> m_requested_surface_formats{ { 
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		} };
	const std::vector<VkPresentModeKHR> m_requested_present_modes{ { VK_PRESENT_MODE_FIFO_KHR } };

	bool is_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface) noexcept;
	bool check_features_support(VkPhysicalDevice physical_device) noexcept;
	bool check_extensions_support(VkPhysicalDevice physical_device) const noexcept;
	bool check_formats_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface) const noexcept;
	bool check_present_modes_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface) const noexcept;
	VkDeviceQueueCreateInfo create_queue_create_info(uint32_t queue_family_index, const float* priority) const noexcept;
};