#include "rendering/VulkanShader.h"
#include <fstream>
#include <vector>

VulkanShader::VulkanShader(VkDevice device, const std::string& filename)
	: m_device{ device }
{
	std::ifstream file{ filename, std::ios::ate | std::ios::binary };
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file " + filename);
	}
	size_t filesize = (size_t)file.tellg();
	std::vector<char> buffer(filesize);
	file.seekg(0);
	file.read(buffer.data(), filesize);
	file.close();
	VkShaderModuleCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = buffer.size();
	info.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
	if (vkCreateShaderModule(device, &info, nullptr, &m_shader_module) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader!");
	}
}

VkShaderModule VulkanShader::get_module() const noexcept
{
	return m_shader_module;
}

void VulkanShader::cleanup() noexcept
{
	if (m_shader_module != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(m_device, m_shader_module, nullptr);
	}
}