#include "rendering/VulkanRenderPass.h"

VulkanRenderPass::VulkanRenderPass(VkDevice device)
	: m_device{ device }
{

}

VkResult VulkanRenderPass::create(VkFormat format) noexcept
{
	VkRenderPassCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	VkAttachmentDescription attachment_description{};
	attachment_description.flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
	attachment_description.format = format;
	attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_NONE;
	attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE;
	attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref{};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	info.attachmentCount = 1;
	info.pAttachments = &attachment_description;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;

	VkResult result{ vkCreateRenderPass(m_device, &info, nullptr, &m_render_pass) };
	return result;
}

std::vector<VkResult> VulkanRenderPass::create_framebuffers(const std::vector<VkImageView>& image_views, const VkExtent2D extent) noexcept
{
	std::vector<VkResult> results;
	m_frame_buffers.resize(image_views.size());
	for (size_t i{ 0 }; i < image_views.size(); i++)
	{
		VkImageView attachments[] = { image_views[i] };
		VkFramebufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.width = extent.width;
		info.height = extent.height;
		info.layers = 1;
		info.renderPass = m_render_pass;
		info.attachmentCount = 1;
		info.pAttachments = attachments;

		VkResult result{ vkCreateFramebuffer(m_device, &info, nullptr, &m_frame_buffers[i]) };
		results.push_back(result);
	}
	return results;
}

void VulkanRenderPass::cleanup() noexcept
{
	if (m_render_pass != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE)
	{
		for (size_t i{ 0 }; i < m_frame_buffers.size(); i++)
		{
			vkDestroyFramebuffer(m_device, m_frame_buffers[i], nullptr);
		}
		vkDestroyRenderPass(m_device, m_render_pass, nullptr);
		m_frame_buffers.clear();
		m_render_pass = VK_NULL_HANDLE;
	}
	
}

