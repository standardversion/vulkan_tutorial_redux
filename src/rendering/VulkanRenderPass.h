#pragma once
#include "vulkan/vulkan.h"
#include <vector>

class VulkanRenderPass
{
public:
	VulkanRenderPass(VkDevice device);
	VkResult create(VkFormat format) noexcept;
	std::vector<VkResult> create_framebuffers(const std::vector<VkImageView>& image_views, const VkExtent2D extent) noexcept;
	VkRenderPass get_render_pass() const noexcept;
	void cleanup() noexcept;
private:
	VkDevice m_device{ VK_NULL_HANDLE };
	VkRenderPass m_render_pass{ VK_NULL_HANDLE };
	std::vector<VkFramebuffer> m_frame_buffers;
};