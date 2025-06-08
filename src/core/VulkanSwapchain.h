#pragma once
#include "vulkan/vulkan.h"
#include <vector>

class VulkanSwapchain
{
public:
	VulkanSwapchain(VkPhysicalDevice physical_device, VkDevice device);
	VkResult create_swapchain(
		VkSurfaceKHR surface,
		VkSurfaceFormatKHR surface_format,
		VkPresentModeKHR present_mode,
		std::vector<uint32_t> queue_family_indices
	) noexcept;
	void cleanup() noexcept;
private:
	VkPhysicalDevice m_physical_device{ VK_NULL_HANDLE };
	VkDevice m_device{ VK_NULL_HANDLE };
	VkExtent2D m_extent;
	VkSwapchainKHR m_swapchain{ VK_NULL_HANDLE };
};