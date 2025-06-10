#include "core/VulkanInstance.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <unordered_set>
#include <iostream>

static constexpr bool ENABLE_VALIDATION_LAYERS =
#ifdef NDEBUG
	false;
#else
	true;
#endif


VulkanInstance::VulkanInstance(const VulkanCfg& vulkan_cfg)
	: m_config{ vulkan_cfg }
{
}

VulkanInstance::~VulkanInstance()
{
	cleanup();
}

VkResult VulkanInstance::create_instance() noexcept
{
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = m_config.app_info.name.c_str();
	app_info.applicationVersion = VK_MAKE_VERSION(m_config.app_info.major, m_config.app_info.minor, m_config.app_info.patch);
	app_info.pEngineName = m_config.app_info.engine_name.c_str();
	app_info.apiVersion = m_config.app_info.vulkan_api_version;
	app_info.engineVersion = VK_MAKE_VERSION(m_config.app_info.engine_major, m_config.app_info.engine_minor, m_config.app_info.engine_patch);

	std::unordered_set<std::string> layer_names{ get_instance_layers_to_enable() };
	std::vector<const char*> enabled_layer_names{};
	enabled_layer_names.reserve(layer_names.size());
	for (const auto& layer : layer_names)
	{
		enabled_layer_names.push_back(layer.c_str());
	}
	std::unordered_set<std::string> ext_names{ get_instance_extensions_to_enable() };
	std::vector<const char*> enabled_ext_names{};
	enabled_ext_names.reserve(enabled_ext_names.size());
	for (const auto& ext : ext_names)
	{
		enabled_ext_names.push_back(ext.c_str());
	}
	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.flags = 0;
	create_info.pApplicationInfo = &app_info;
	create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layer_names.size());
	create_info.ppEnabledLayerNames = enabled_layer_names.data();
	create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_ext_names.size());
	create_info.ppEnabledExtensionNames = enabled_ext_names.data();

	if (ENABLE_VALIDATION_LAYERS)
	{
		VkDebugUtilsMessengerCreateInfoEXT debug_msg_info{};
		populate_debug_messenger_create_info(debug_msg_info);
		create_info.pNext = &debug_msg_info;
	}

	return vkCreateInstance(&create_info, nullptr, &m_instance);
}

void VulkanInstance::init(GLFWwindow* window)
{
	if (create_instance() != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan Instance!");
	}

	if (setup_debug_messenger() != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to setup debug messenger");
	}

	if (create_surface(window) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create surface!");
	}

	m_device = std::make_unique<VulkanDevice>();
	if (m_device->pick_physical_device(m_instance, m_surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to pick a physical device!");
	}
	if (m_device->create_logical_device(m_instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a logical device!");
	}

	VkPhysicalDevice physical_device{ m_device->get_physical_device() };
	VkDevice device{ m_device->get_logical_device() };
	m_swapchain = std::make_unique<VulkanSwapchain>(physical_device, device, m_config.command_pool);
	VkSurfaceFormatKHR surface_format{ m_device->pick_format() };
	VkPresentModeKHR present_mode{ m_device->pick_present_mode() };
	std::vector<uint32_t> queue_family_indices{m_device->get_queue_family_indices().get_indices()};
	if (m_swapchain->create(m_surface, surface_format, present_mode, queue_family_indices) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swapchain!");
	}
	m_command_pool = std::make_unique<VulkanCommandPool>(device, queue_family_indices, m_config.command_pool);
	std::unordered_map<uint32_t, VkResult> create_pool_results{ m_command_pool->create() };
	for (const auto& [index, result] : create_pool_results)
	{
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool for queue index: " + std::to_string(index));
		}
	}
	std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkResult>> allocate_buffers_results{
		m_command_pool->allocate_buffers() 
	};
	for (const auto& [queue_index, cmd_buffers_by_frame] : allocate_buffers_results)
	{
		for (const auto& [frame_index, result] : cmd_buffers_by_frame)
		{
			if (result != VK_SUCCESS)
			{
				throw std::runtime_error(
					"Failed to allocate buffers for queue index: "
					+ std::to_string(queue_index)
					+ " frame index: "
					+ std::to_string(frame_index)
				);
			}
		}
	}
}

std::vector<VkLayerProperties> VulkanInstance::get_instance_layer_properties() noexcept
{
	uint32_t prop_count{};
	vkEnumerateInstanceLayerProperties(&prop_count, nullptr);
	std::vector<VkLayerProperties> layer_props(prop_count);
	vkEnumerateInstanceLayerProperties(&prop_count, layer_props.data());
	return layer_props;
}

std::unordered_set<std::string> VulkanInstance::get_instance_layers_to_enable() const
{
	std::vector<VkLayerProperties> layer_props{ get_instance_layer_properties() };
	std::unordered_set<std::string> layer_names;
	for (const auto& req_layer : m_requested_validation_layers)
	{
		bool found{ false };
		for (const auto& prop : layer_props)
		{
			if (ENABLE_VALIDATION_LAYERS && strcmp(prop.layerName, req_layer) == 0)
			{
				found = true;
				layer_names.insert(prop.layerName);
			}
		}
		if (ENABLE_VALIDATION_LAYERS && !found)
		{
			throw std::runtime_error("Validation layer: " + std::string(req_layer) + " not found");
		}
	}
	return layer_names;
}

std::vector<VkExtensionProperties> VulkanInstance::get_instance_extension_properties() noexcept
{
	uint32_t prop_count{};
	vkEnumerateInstanceExtensionProperties(nullptr, &prop_count, nullptr);
	std::vector<VkExtensionProperties> ext_props(prop_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &prop_count, ext_props.data());
	return ext_props;
}

std::unordered_set<std::string> VulkanInstance::get_instance_extensions_to_enable() const
{
	std::vector<VkExtensionProperties> ext_props{ get_instance_extension_properties() };
	uint32_t glfw_ext_count{};
	const char** glfw_req_exts{ glfwGetRequiredInstanceExtensions(&glfw_ext_count) };
	std::unordered_set<std::string> ext_names;
	for (uint32_t i{ 0 }; i < glfw_ext_count; i++)
	{	
		bool found{ false };
		for (const auto& prop : ext_props)
		{
			if (strcmp(glfw_req_exts[i], prop.extensionName) == 0)
			{
				found = true;
				ext_names.insert(prop.extensionName);
			}
		}
		if (!found)
		{
			throw std::runtime_error(std::string("Extension not found: ") + glfw_req_exts[i]);
		}
	}

	if (ENABLE_VALIDATION_LAYERS)
	{
		for (const auto& req_ext : m_requested_debug_extensions)
		{
			bool found{ false };
			for (const auto& prop : ext_props)
			{
				if (strcmp(req_ext, prop.extensionName) == 0)
				{
					found = true;
					ext_names.insert(prop.extensionName);
				}
			}
			if (!found)
			{
				throw std::runtime_error("Debug extension not found: " + std::string(req_ext));
			}
		}
	}
	return ext_names;
}

void VulkanInstance::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& info) const noexcept
{
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	info.flags = 0;
	info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	info.pfnUserCallback = debug_callback;
}

VkResult VulkanInstance::setup_debug_messenger()
{
	if (!ENABLE_VALIDATION_LAYERS) return VK_SUCCESS;
	auto func{ (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT") };
	VkDebugUtilsMessengerCreateInfoEXT info;
	populate_debug_messenger_create_info(info);
	return func(m_instance, &info, nullptr, &m_debug_messenger);
}

void VulkanInstance::destroy_debug_messenger() noexcept
{
	if (!ENABLE_VALIDATION_LAYERS) return;
	auto func{ (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT") };
	func(m_instance, m_debug_messenger, nullptr);
	m_debug_messenger = VK_NULL_HANDLE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_types,
	const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
	void* p_user_data
)
{
	std::cerr << "validation layer: " << p_callback_data->pMessage << std::endl;

	return VK_FALSE;
}

VkResult VulkanInstance::create_surface(GLFWwindow* window) noexcept
{
	return glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface);
}

VkInstance VulkanInstance::get() const noexcept
{
	return m_instance;
}

void VulkanInstance::cleanup() noexcept
{
	if (m_instance != VK_NULL_HANDLE)
	{
		m_command_pool->cleanup();
		m_swapchain->cleanup();
		m_device->cleanup();
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		destroy_debug_messenger();
		vkDestroyInstance(m_instance, nullptr);
		m_instance = VK_NULL_HANDLE;
	}
}