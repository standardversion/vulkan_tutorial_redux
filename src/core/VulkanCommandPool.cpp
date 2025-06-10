#include "core/VulkanCommandPool.h"
#include "app/Config.h"

VulkanCommandPool::VulkanCommandPool(VkDevice device, const std::vector<uint32_t>& queue_family_indices, const CommandPoolCfg& command_pool_cfg)
	: m_device{ device }, m_config{ command_pool_cfg }
{ 
	for (size_t i{ 0 }; i < queue_family_indices.size(); i++)
	{
		VkCommandPool pool{ VK_NULL_HANDLE };
		m_pools_by_family[queue_family_indices[i]] = pool;
	}
	m_cmd_buffers.reserve(m_pools_by_family.size());
	m_cmd_buffer_states.reserve(m_pools_by_family.size());
}

std::unordered_map<uint32_t, VkResult> VulkanCommandPool::create() noexcept
{
	std::unordered_map<uint32_t, VkResult> results;
	for (auto& [index, pool] : m_pools_by_family)
	{
		VkCommandPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		create_info.queueFamilyIndex = index;

		VkResult result{ vkCreateCommandPool(m_device, &create_info, nullptr, &m_pools_by_family[index])};
		results[index] = result;
	}
	return results;
}

std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkResult>> VulkanCommandPool::allocate_buffers() noexcept
{
	std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkResult>> results;
	for (auto& [queue_index, pool] : m_pools_by_family)
	{
		std::unordered_map<uint32_t, std::vector<VkCommandBuffer>> cmd_buffers_by_frame;
		std::unordered_map<uint32_t, std::vector<CommandBufferState>> cmd_buffer_states_by_frame;
		for (uint32_t frame_index{ 0 }; frame_index < m_config.max_frames_in_flight; frame_index++)
		{
			VkCommandBufferAllocateInfo allocate_info{};
			allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocate_info.commandBufferCount = m_config.buffers_per_frame;
			allocate_info.commandPool = m_pools_by_family[queue_index];
			allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmd_buffers_by_frame[frame_index].resize(m_config.buffers_per_frame);
			cmd_buffer_states_by_frame[frame_index].resize(m_config.buffers_per_frame, CommandBufferState::NotAllocated);
			VkResult result{ vkAllocateCommandBuffers(m_device, &allocate_info, cmd_buffers_by_frame[frame_index].data())};
			results[queue_index][frame_index] = result;
			if (result != VK_SUCCESS)
			{
				free_buffers_for_frame(queue_index, frame_index);
			}
			else
			{
				for (size_t i{ 0 }; i < m_config.buffers_per_frame; i++)
				{
					cmd_buffer_states_by_frame[frame_index][i] = CommandBufferState::Ready;
				}
			}
		}	
		m_cmd_buffers[queue_index] = cmd_buffers_by_frame;
		m_cmd_buffer_states[queue_index] = cmd_buffer_states_by_frame;
	}
	return results;
}

VkCommandPool VulkanCommandPool::get_pool(uint32_t queue_family_index) const noexcept
{
	if (m_pools_by_family.contains(queue_family_index))
	{
		return m_pools_by_family.at(queue_family_index);
	}
	return VK_NULL_HANDLE;
}

std::span<const VkCommandBuffer> VulkanCommandPool::get_buffers(uint32_t queue_family_index, uint32_t frame_index) const noexcept
{
	if (m_cmd_buffers.contains(queue_family_index) && m_cmd_buffers.at(queue_family_index).contains(frame_index))
	{
		const auto& buffers = m_cmd_buffers.at(queue_family_index).at(frame_index);
		return std::span<const VkCommandBuffer>(buffers.data(), buffers.size());
	}
	return std::span<const VkCommandBuffer>{};
}

VkResult VulkanCommandPool::begin_recording(uint32_t queue_family_index, uint32_t frame_index, uint32_t per_frame_buffer_index) noexcept
{
	if (!m_cmd_buffers.contains(queue_family_index) || !m_cmd_buffers[queue_family_index].contains(frame_index)
		|| per_frame_buffer_index >= m_config.buffers_per_frame
		|| m_cmd_buffer_states[queue_family_index][frame_index][per_frame_buffer_index] != CommandBufferState::Ready)
	{
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VkCommandBuffer buffer{ m_cmd_buffers.at(queue_family_index).at(frame_index)[per_frame_buffer_index]};
	VkResult result{ vkBeginCommandBuffer(buffer, &begin_info) };
	if (result == VK_SUCCESS)
	{
		m_cmd_buffer_states[queue_family_index][frame_index][per_frame_buffer_index] = CommandBufferState::Recording;
	}
	return result;
}

VkResult VulkanCommandPool::end_recording(uint32_t queue_family_index, uint32_t frame_index, uint32_t per_frame_buffer_index) noexcept
{
	if (!m_cmd_buffers.contains(queue_family_index) || !m_cmd_buffers[queue_family_index].contains(frame_index)
		|| per_frame_buffer_index >= m_config.buffers_per_frame
		|| m_cmd_buffer_states[queue_family_index][frame_index][per_frame_buffer_index] != CommandBufferState::Recording)
	{
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	VkCommandBuffer buffer{ m_cmd_buffers.at(queue_family_index).at(frame_index)[per_frame_buffer_index] };
	VkResult result{ vkEndCommandBuffer(buffer) };
	if (result == VK_SUCCESS)
	{
		m_cmd_buffer_states[queue_family_index][frame_index][per_frame_buffer_index] = CommandBufferState::RecordingEnded;
	}
	return result;
}

CommandBufferState VulkanCommandPool::get_state(uint32_t queue_index, uint32_t frame_index, uint32_t buffer_index) const noexcept
{
	if (m_cmd_buffer_states.contains(queue_index) &&
		m_cmd_buffer_states.at(queue_index).contains(frame_index) &&
		buffer_index < m_cmd_buffer_states.at(queue_index).at(frame_index).size())
	{
		return m_cmd_buffer_states.at(queue_index).at(frame_index)[buffer_index];
	}
	return CommandBufferState::NotAllocated;
}

std::vector<VkResult> VulkanCommandPool::reset_cmd_buffers(uint32_t queue_family_index, uint32_t frame_index) noexcept
{
	std::vector<VkResult> results;
	if (!m_cmd_buffers.contains(queue_family_index) || !m_cmd_buffers[queue_family_index].contains(frame_index))
	{
		return results;
	}

	auto& buffers = m_cmd_buffers.at(queue_family_index).at(frame_index);
	for (size_t i = 0; i < buffers.size(); ++i)
	{
		VkResult result = vkResetCommandBuffer(buffers[i], 0);
		if (result == VK_SUCCESS)
		{
			m_cmd_buffer_states[queue_family_index][frame_index][i] = CommandBufferState::Ready;
		}
		results.push_back(result);
	}
	return results;
}


void VulkanCommandPool::free_buffers_for_frame(uint32_t queue_family_index, uint32_t frame_index) noexcept
{
	if (m_pools_by_family.contains(queue_family_index)
		&& m_cmd_buffers.contains(queue_family_index)
		&& m_cmd_buffers[queue_family_index].contains(frame_index))
	{
		auto& buffers = m_cmd_buffers.at(queue_family_index).at(frame_index);
		if (!buffers.empty())
		{
			vkFreeCommandBuffers(m_device, m_pools_by_family.at(queue_family_index),
				static_cast<uint32_t>(buffers.size()), buffers.data());
			m_cmd_buffers[queue_family_index][frame_index].clear();
		}
	}
}

void VulkanCommandPool::cleanup() noexcept
{
	for (auto& [queue_index, pool] : m_pools_by_family)
	{
		if (pool != VK_NULL_HANDLE)
		{
			for (uint32_t frame_index{ 0 }; frame_index < m_config.max_frames_in_flight; frame_index++)
			{
				free_buffers_for_frame(queue_index, frame_index);
			}
			vkDestroyCommandPool(m_device, pool, nullptr);
		}
	}
	m_pools_by_family.clear();
}