#include "core/VulkanDevice.h"
#include <vector>
#include <stdexcept>
#include <iostream>
#include <unordered_set>

constexpr uint32_t QUEUE_INDEX = 0;

void VulkanDevice::pick_physical_device(VkInstance instance, VkSurfaceKHR surface)
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
		throw std::runtime_error("Failed to find physical device!");
	}
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
	std::unordered_set<VkFormat> available_formats;
	for (const auto& surface_format : surface_formats)
	{
		available_formats.insert(surface_format.format);
	}
	for (const auto& format : m_requested_surface_formats)
	{
		if (!available_formats.contains(format)) return false;
	}
	return true;
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

void VulkanDevice::create_logical_device(VkInstance instance)
{
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	std::unordered_set<uint32_t> unique_queue_families = { m_queue_families.graphics.value(), m_queue_families.present.value() };
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
	
	if (vkCreateDevice(m_physical_device, &device_create_info, nullptr, &m_device) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device!");
	}

	vkGetDeviceQueue(m_device, m_queue_families.graphics.value(), QUEUE_INDEX, &m_graphics_queue);
	vkGetDeviceQueue(m_device, m_queue_families.present.value(), QUEUE_INDEX, &m_present_queue);
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