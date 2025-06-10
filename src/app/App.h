#pragma once
#include "core/VulkanInstance.h"
#include "app/Config.h"
#include <cstdint>
#include <string>
#include <memory>

class GLFWwindow;

class App
{
public:
	App(const Config& config);
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	~App();
	void init();
	void run();

	static void key_callback(GLFWwindow* m_window, int key, int scancode, int action, int mods);

private:
	Config m_config;
	GLFWwindow* m_window{nullptr};
	std::unique_ptr<VulkanInstance> m_instance;

	void cleanup() noexcept;
};