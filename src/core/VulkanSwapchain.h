#pragma once
#include "app/Config.h"
#include "vulkan/vulkan.h"
#include <vector>

class VulkanSwapchain
{
public:
	VulkanSwapchain(VkPhysicalDevice physical_device, VkDevice device, const CommandPoolCfg& command_pool_cfg);
	VkResult create(
		VkSurfaceKHR surface,
		VkSurfaceFormatKHR surface_format,
		VkPresentModeKHR present_mode,
		std::vector<uint32_t> queue_family_indices
	) noexcept;
	void cleanup() noexcept;
private:
	VkPhysicalDevice m_physical_device{ VK_NULL_HANDLE };
	VkDevice m_device{ VK_NULL_HANDLE };
	CommandPoolCfg m_config{};
	VkExtent2D m_extent;
	VkSwapchainKHR m_swapchain{ VK_NULL_HANDLE };
};