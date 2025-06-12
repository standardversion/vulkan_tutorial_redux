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
	if (result == VK_SUCCESS)
	{
		uint32_t swapchain_image_count;
		vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, nullptr);
		m_images.resize(swapchain_image_count);
		vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, m_images.data());
		m_image_views.resize(swapchain_image_count);
	}
	return result;
}

std::vector<VkResult> VulkanSwapchain::create_image_views(VkSurfaceFormatKHR surface_format) noexcept
{
	std::vector<VkResult> results;
	VkComponentMapping components{
		.r =VK_COMPONENT_SWIZZLE_R,
		.g = VK_COMPONENT_SWIZZLE_G,
		.b = VK_COMPONENT_SWIZZLE_B,
		.a = VK_COMPONENT_SWIZZLE_A
	};
	VkImageSubresourceRange subresource_range{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};
	for (size_t i{ 0 }; i < m_images.size(); i++)
	{
		VkImageViewCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.flags = 0;
		info.image = m_images[i];
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = surface_format.format;
		info.components = components;
		info.subresourceRange = subresource_range;

		results.push_back(vkCreateImageView(m_device, &info, nullptr, &m_image_views[i]));
	}
	return results;
}

const std::vector<VkImageView>& VulkanSwapchain::get_image_views() const noexcept
{
	return m_image_views;
}

const VkExtent2D VulkanSwapchain::get_extent() const noexcept
{
	return m_extent;
}

void VulkanSwapchain::cleanup() noexcept
{
	if (m_swapchain != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE)
	{
		for (auto& image_view : m_image_views)
		{
			vkDestroyImageView(m_device, image_view, nullptr);
		}
		vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
		m_image_views.clear();
		m_images.clear();
		m_swapchain = VK_NULL_HANDLE;

	}
}