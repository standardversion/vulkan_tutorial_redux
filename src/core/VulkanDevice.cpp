#include "core/VulkanDevice.h"
#include <vector>
#include <stdexcept>
#include <iostream>
#include <unordered_set>

constexpr uint32_t QUEUE_INDEX = 0;

std::vector<uint32_t> QueueFamilyIndices::get_indices() const noexcept
{
	std::vector<uint32_t> indices;
	if (graphics.has_value()) indices.push_back(graphics.value());
	if (present.has_value() && present != graphics) indices.push_back(present.value());
	return indices;
}

VkResult VulkanDevice::pick_physical_device(VkInstance instance, VkSurfaceKHR surface) noexcept
{
	uint32_t physical_device_count{};
	vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
	std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
	vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

	for (const auto& physical_device : physical_devices)
	{
		if (!is_device_suitable(physical_device, surface)) continue;

		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physical_device, &props);

		uint32_t queue_family_props_count;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_props_count, nullptr);
		std::vector<VkQueueFamilyProperties> queue_family_props(queue_family_props_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_props_count, queue_family_props.data());
		for (size_t i{ 0 }; i < queue_family_props.size(); i++)
		{
			if (!m_queue_families.graphics.has_value() && (queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				m_queue_families.graphics = i;
			}

			if (!m_queue_families.present.has_value())
			{
				VkBool32 supported{ false };
				vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, static_cast<uint32_t>(i), surface, &supported);
				if (supported)
				{
					m_queue_families.present = i;
				}
			}
			if (m_queue_families.graphics.has_value() && m_queue_families.present.has_value())
			{
				break;
			}
		}

		if (m_queue_families.graphics.has_value() && m_queue_families.present.has_value())
		{
			m_physical_device = physical_device;
			std::cout << "Selected GPU: " << props.deviceName << '\n';
			break;
		}
	}
	
	if (m_physical_device == VK_NULL_HANDLE)
	{
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	return VK_SUCCESS;
}

bool VulkanDevice::is_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface) noexcept
{
	if (!check_features_support(physical_device)) return false;
	if (!check_extensions_support(physical_device)) return false;
	if (!check_formats_support(physical_device, surface)) return false;
	if (!check_present_modes_support(physical_device, surface)) return false;
	return true;
}

bool VulkanDevice::check_features_support(VkPhysicalDevice physical_device) noexcept
{
	vkGetPhysicalDeviceFeatures(physical_device, &m_features);
	return m_features.samplerAnisotropy && m_features.geometryShader;
}

bool VulkanDevice::check_extensions_support(VkPhysicalDevice physical_device) const noexcept
{
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> extension_properties(extension_count);
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extension_properties.data());
	std::unordered_set<std::string> available_extensions;
	for (const auto& extension_property : extension_properties)
	{
		available_extensions.insert(extension_property.extensionName);
	}
	for (const auto& ext : m_requested_extensions)
	{
		if (!available_extensions.contains(ext)) return false;
	}
	return true;
}

bool VulkanDevice::check_formats_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface) const noexcept
{
	uint32_t surface_format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, nullptr);
	std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_formats.data());
	for (const auto& requested_format : m_requested_surface_formats)
	{
		for (const auto& available_format : surface_formats)
		{
			if (available_format.format == requested_format.format
				&& available_format.colorSpace == requested_format.colorSpace)
			{
				return true;
			}
		}
	}
	return false;
}

bool VulkanDevice::check_present_modes_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface) const noexcept
{
	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
	std::vector<VkPresentModeKHR> present_modes(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());
	std::unordered_set<VkPresentModeKHR> available_modes;
	for (const auto& present_mode : present_modes)
	{
		available_modes.insert(present_mode);
	}
	for (const auto& mode : m_requested_present_modes)
	{
		
		if (!available_modes.contains(mode)) return false;
	}
	return true;
}

VkResult VulkanDevice::create_logical_device(VkInstance instance) noexcept
{
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	std::vector<uint32_t> unique_queue_families{ m_queue_families.get_indices() };
	// Maintain the actual priority values here (ensures valid memory lifetime)
	std::vector<float> queue_priorities(unique_queue_families.size(), 1.0f);
	size_t i = 0;
	for (uint32_t index : unique_queue_families)
	{
		queue_create_infos.push_back(create_queue_create_info(index, &queue_priorities[i++]));
	}

	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.enabledExtensionCount = static_cast<uint32_t>(m_requested_extensions.size());
	device_create_info.ppEnabledExtensionNames = m_requested_extensions.data();
	device_create_info.pEnabledFeatures = &m_features;
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
	device_create_info.pQueueCreateInfos = queue_create_infos.data();
	
	VkResult result{ vkCreateDevice(m_physical_device, &device_create_info, nullptr, &m_device) };
	if (result != VK_SUCCESS) return result;

	vkGetDeviceQueue(m_device, m_queue_families.graphics.value(), QUEUE_INDEX, &m_graphics_queue);
	vkGetDeviceQueue(m_device, m_queue_families.present.value(), QUEUE_INDEX, &m_present_queue);
	return VK_SUCCESS;
}

VkDeviceQueueCreateInfo VulkanDevice::create_queue_create_info(uint32_t queue_family_index, const float* priority) const noexcept
{
	VkDeviceQueueCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	info.queueFamilyIndex = queue_family_index;
	info.flags = NULL;
	info.queueCount = 1;
	info.pQueuePriorities = priority;
	return info;
}

void VulkanDevice::cleanup()
{
	if (m_device != VK_NULL_HANDLE)
	{
		vkDestroyDevice(m_device, nullptr);
		m_device = VK_NULL_HANDLE;
	}
}

VkPhysicalDevice VulkanDevice::get_physical_device() const noexcept
{
	return m_physical_device;
}

VkDevice VulkanDevice::get_logical_device() const noexcept
{
	return m_device;
}

QueueFamilyIndices VulkanDevice::get_queue_family_indices() const noexcept
{
	return m_queue_families;
}

VkSurfaceFormatKHR VulkanDevice::pick_format(const std::vector<VkSurfaceFormatKHR >& preferred_formats) const noexcept
{
	for (const auto& format : preferred_formats)
	{
		for (const auto& available_format : m_requested_surface_formats)
		{
			if (available_format.format == format.format && available_format.colorSpace == format.colorSpace)
			{
				return format;
			}
		}
	}
	return m_requested_surface_formats[0];
}

VkSurfaceFormatKHR VulkanDevice::pick_format() const noexcept
{
	return m_requested_surface_formats[0];
}

VkPresentModeKHR VulkanDevice::pick_present_mode(const std::vector<VkPresentModeKHR>& preferred_modes) const noexcept
{
	for (const auto& mode : preferred_modes)
	{
		if (std::find(
			m_requested_present_modes.begin(), m_requested_present_modes.end(), mode
		) != m_requested_present_modes.end())
		{
			return mode;
		}
	}
	return m_requested_present_modes[0];
}

VkPresentModeKHR VulkanDevice::pick_present_mode() const noexcept
{

	return m_requested_present_modes[0];
}