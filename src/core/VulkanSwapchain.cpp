#include "core/VulkanSwapchain.h"
#include "app/Config.h"
#include <stdexcept>

VulkanSwapchain::VulkanSwapchain(VkPhysicalDevice physical_device, VkDevice device, const CommandPoolCfg& command_pool_cfg)
	: m_physical_device{ physical_device }, m_device{ device }, m_config{ command_pool_cfg }
{ }


VkResult VulkanSwapchain::create(
	VkSurfaceKHR surface,
	VkSurfaceFormatKHR surface_format,
	VkPresentModeKHR present_mode,
	std::vector<uint32_t> queue_family_indices
) noexcept
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, surface, &capabilities);
	m_extent = capabilities.currentExtent;
	VkSharingMode sharing_mode{ queue_family_indices.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE };
	VkSwapchainCreateInfoKHR create_info{};
	uint32_t image_count = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
		image_count = capabilities.maxImageCount;
	}
	//This ensures the swapchain has enough images to keep up with your rendering pipeline
	image_count = std::max(image_count, m_config.max_frames_in_flight);
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.flags = NULL;
	create_info.surface = surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.presentMode = present_mode;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = m_extent;
	create_info.imageArrayLayers = 1;
	create_info.imageSharingMode = sharing_mode;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.preTransform = capabilities.currentTransform;
	create_info.clipped = VK_TRUE;

	if (sharing_mode == VK_SHARING_MODE_CONCURRENT)
	{
		create_info.pQueueFamilyIndices = queue_family_indices.data();
		create_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size());
	}

	VkResult result{ vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain) };
	return result;
}

void VulkanSwapchain::cleanup() noexcept
{
	if (m_swapchain != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
		m_swapchain = VK_NULL_HANDLE;
	}
	
}