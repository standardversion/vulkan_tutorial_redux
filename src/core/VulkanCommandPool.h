#pragma once
#include "app/Config.h"
#include "vulkan/vulkan.h"
#include <vector>
#include <unordered_map>
#include <span>

enum class CommandBufferState {
	NotAllocated,
	Ready,          // Allocated but not recording
	Recording,
	RecordingEnded
};


class VulkanCommandPool
{
public:
	VulkanCommandPool(VkDevice device, const std::vector<uint32_t>& queue_family_indices, const CommandPoolCfg& command_pool_cfg);
	std::unordered_map<uint32_t, VkResult> create() noexcept;
	std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkResult>> allocate_buffers() noexcept;
	VkCommandPool get_pool(uint32_t queue_family_index) const noexcept;
	std::span<const VkCommandBuffer> get_buffers(uint32_t queue_family_index, uint32_t frame_index) const noexcept;
	std::vector<VkResult> reset_cmd_buffers(uint32_t queue_family_index, uint32_t frame_index) noexcept;
	VkResult begin_recording(uint32_t queue_family_index, uint32_t frame_index, uint32_t per_frame_buffer_index) noexcept;
	VkResult end_recording(uint32_t queue_family_index, uint32_t frame_index, uint32_t per_frame_buffer_index) noexcept;
	CommandBufferState get_state(uint32_t queue_index, uint32_t frame_index, uint32_t buffer_index) const noexcept;
	VkResult submit(
		VkQueue queue,
		uint32_t queue_index,
		uint32_t frame_index,
		uint32_t buffer_index,
		const std::vector<VkSemaphore>& wait_semaphores,
		const std::vector<VkPipelineStageFlags>& wait_stages,
		const std::vector<VkSemaphore>& signal_semaphores,
		VkFence fence
	) noexcept;
	void cleanup() noexcept;
private:
	VkDevice m_device{ VK_NULL_HANDLE };
	CommandPoolCfg m_config{};
	std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::vector<VkCommandBuffer>>> m_cmd_buffers;
	std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::vector<CommandBufferState>>> m_cmd_buffer_states;
	std::unordered_map<uint32_t, VkCommandPool> m_pools_by_family;

	void free_buffers_for_frame(uint32_t queue_family_index, uint32_t frame_index) noexcept;
};