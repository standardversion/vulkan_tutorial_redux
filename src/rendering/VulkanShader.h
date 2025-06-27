#pragma once
#include "vulkan/vulkan.h"
#include <string>

class VulkanShader
{
public:
	VulkanShader(VkDevice device, const std::string& filename);
	VkShaderModule get_module() const noexcept;
	void cleanup() noexcept;
private:
	VkDevice m_device{ VK_NULL_HANDLE };
	VkShaderModule m_shader_module{ VK_NULL_HANDLE };
};