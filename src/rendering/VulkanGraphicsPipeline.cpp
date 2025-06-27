#include "rendering/VulkanGraphicsPipeline.h"
#include "rendering/VulkanShader.h"

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VkDevice device)
	: m_device{ device }
{ }

VkResult VulkanGraphicsPipeline::create(VkExtent2D extent, VkRenderPass renderpass) noexcept
{
	VkPipelineLayoutCreateInfo layout_create_info{};
	layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_create_info.pPushConstantRanges = nullptr;
	layout_create_info.pushConstantRangeCount = 0;
	layout_create_info.pSetLayouts = nullptr;
	layout_create_info.setLayoutCount = 0;

	vkCreatePipelineLayout(m_device, &layout_create_info, nullptr, &m_pipeline_layout);
	

	VulkanShader vert_shader{ m_device, "vert.spv" };
	VulkanShader frag_shader{ m_device, "frag.spv" };

	VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
	vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader.get_module();
	vert_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
	frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader.get_module();
	frag_shader_stage_info.pName = "main";

	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos{ vert_shader_stage_info, frag_shader_stage_info };

	// vertex input state
	VkPipelineVertexInputStateCreateInfo vert_input_state_info{};
	vert_input_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vert_input_state_info.pVertexBindingDescriptions = nullptr;
	vert_input_state_info.vertexBindingDescriptionCount = 0;
	vert_input_state_info.pVertexAttributeDescriptions = nullptr;
	vert_input_state_info.vertexAttributeDescriptionCount = 0;

	// primitive assembly state
	VkPipelineInputAssemblyStateCreateInfo input_assemble_state_info{};
	input_assemble_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assemble_state_info.primitiveRestartEnable = false;
	input_assemble_state_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	//viewport
	VkViewport viewport{};
	viewport.width = extent.width;
	viewport.height = extent.height;
	viewport.x = 0;
	viewport.y = 0;
	viewport.minDepth = 0.01;
	viewport.maxDepth = 1.0;
	VkOffset2D offset{};
	offset.x = 0;
	offset.y = 0;
	VkRect2D scissors{};
	scissors.extent = extent;
	scissors.offset = offset;
	VkPipelineViewportStateCreateInfo viewport_state_info{};
	viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_info.scissorCount = 1;
	viewport_state_info.pScissors = &scissors;
	viewport_state_info.viewportCount = 1;
	viewport_state_info.pViewports = &viewport;

	//rasterization
	VkPipelineRasterizationStateCreateInfo rasterization_state_info{};
	rasterization_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state_info.depthClampEnable = false;
	rasterization_state_info.rasterizerDiscardEnable = false;
	rasterization_state_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_state_info.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterization_state_info.frontFace = VK_FRONT_FACE_CLOCKWISE;

	//multisample
	VkPipelineMultisampleStateCreateInfo multisample_state_info{};
	multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisample_state_info.sampleShadingEnable = false;

	//colorblending
	VkPipelineColorBlendAttachmentState colorblend_attachement{};
	colorblend_attachement.blendEnable = false;
	VkPipelineColorBlendStateCreateInfo colorblend_state_info{};
	colorblend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorblend_state_info.attachmentCount = 1;
	colorblend_state_info.pAttachments = &colorblend_attachement;

	VkGraphicsPipelineCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.pStages = shader_stage_infos.data();
	create_info.stageCount = shader_stage_infos.size();
	create_info.pVertexInputState = &vert_input_state_info;
	create_info.pInputAssemblyState = &input_assemble_state_info;
	create_info.pViewportState = &viewport_state_info;
	create_info.pRasterizationState = &rasterization_state_info;
	create_info.pMultisampleState = &multisample_state_info;
	create_info.pColorBlendState = &colorblend_state_info;
	create_info.renderPass = renderpass;
	create_info.subpass = 0;
	create_info.layout = m_pipeline_layout;
	m_pipelines.resize(1);
	VkResult result{ vkCreateGraphicsPipelines(m_device, nullptr, 1, &create_info, nullptr, m_pipelines.data()) };
	vkDestroyShaderModule(m_device, vert_shader.get_module(), nullptr);
	vkDestroyShaderModule(m_device, frag_shader.get_module(), nullptr);
	return result;
}

void VulkanGraphicsPipeline::cleanup() noexcept
{
	if (m_pipeline_layout != VK_NULL_HANDLE && m_pipelines.size())
	{
		vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
		m_pipeline_layout = VK_NULL_HANDLE;
		for (auto& pipeline : m_pipelines)
		{
			vkDestroyPipeline(m_device, pipeline, nullptr);
		}
	}
	
}