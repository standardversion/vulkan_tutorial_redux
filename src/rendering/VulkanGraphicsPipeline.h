#pragma once
#include "vulkan/vulkan.h"
#include <vector>

class VulkanGraphicsPipeline
{
public:
	VulkanGraphicsPipeline(VkDevice device);
	VkResult create(VkExtent2D extent, VkRenderPass renderpass) noexcept;
	void cleanup() noexcept;
private:
	VkDevice m_device{ VK_NULL_HANDLE };
	VkPipelineLayout m_pipeline_layout{ VK_NULL_HANDLE };
	std::vector<VkPipeline> m_pipelines{};

};