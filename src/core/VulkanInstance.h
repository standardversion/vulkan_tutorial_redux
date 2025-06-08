#pragma once
#include "core/VulkanDevice.h"
#include "core/VulkanSwapchain.h"
#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>
#include <string>
#include <string_view>
#include <cstdint>
#include <unordered_set>
#include <memory>

class VulkanInstance
{
public:
	VulkanInstance() = default;
	VulkanInstance(const VulkanInstance&) = delete;
	VulkanInstance& operator=(const VulkanInstance&) = delete;
	VulkanInstance(VulkanInstance&&) = delete;
	VulkanInstance& operator=(VulkanInstance&&) = delete;

	VulkanInstance(std::string_view app_name, uint32_t major, uint32_t minor, uint32_t patch);
	~VulkanInstance();
	void init(GLFWwindow* window);
	[[nodiscard]] VkInstance get() const noexcept;
	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_types,
		const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
		void* p_user_data
	);
	
private:
	std::string m_app_name{ "VULKAN APP" };
	uint32_t m_app_version{ VK_API_VERSION_1_0 };
	VkInstance m_instance{ VK_NULL_HANDLE };
	VkDebugUtilsMessengerEXT m_debug_messenger{ VK_NULL_HANDLE };
	const std::vector<const char*> m_requested_validation_layers{ { "VK_LAYER_KHRONOS_validation" } };
	const std::vector<const char*> m_requested_debug_extensions{ { VK_EXT_DEBUG_UTILS_EXTENSION_NAME } };
	VkSurfaceKHR m_surface{ VK_NULL_HANDLE };
	std::unique_ptr<VulkanDevice> m_device;
	std::unique_ptr<VulkanSwapchain> m_swapchain;

	[[nodiscard]] static std::vector<VkLayerProperties> get_instance_layer_properties() noexcept;
	[[nodiscard]] std::unordered_set<std::string> get_instance_layers_to_enable() const;
	[[nodiscard]] static std::vector<VkExtensionProperties> get_instance_extension_properties() noexcept;
	[[nodiscard]] std::unordered_set<std::string> get_instance_extensions_to_enable() const;
	void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& info) const noexcept;
	VkResult create_instance() noexcept;
	VkResult setup_debug_messenger();
	VkResult create_surface(GLFWwindow* window) noexcept;
	void destroy_debug_messenger() noexcept;
	void cleanup() noexcept;
};