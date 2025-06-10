#include "app/App.h"
#include "app/Config.h"
#include <GLFW/glfw3.h>
#include <stdexcept>

App::App(const Config& config)
	: m_config{ config }
{

}

App::~App()
{
	cleanup();
}

void App::init()
{
	if (!glfwInit())
	{
		throw std::runtime_error("Failed to initialize GLFW!");
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	/*
	Last 2 args:
	monitor	The monitor to use for full screen mode, or NULL for windowed mode.
	share	The window whose context to share resources with, or NULL to not share resources.
	*/
	m_window = glfwCreateWindow(m_config.window.width, m_config.window.height, m_config.window.title.c_str(), NULL, NULL);
	if (!m_window)
	{
		throw std::runtime_error("Failed to create GLFW Window!");
	}

	m_instance = std::make_unique<VulkanInstance>(m_config.vulkan_cfg);
	m_instance->init(m_window);
}

void App::run()
{
	glfwSetKeyCallback(m_window, key_callback);
	while (!glfwWindowShouldClose(m_window))
	{
		//Keep running
		glfwPollEvents();
	}
}

void App::cleanup() noexcept
{
	if (m_window) glfwDestroyWindow(m_window);
	glfwTerminate();
}

void App::key_callback(GLFWwindow* m_window, int key, int scancode, int action, int mods)
{
	// If ESC is pressed set the close flag to true on the window
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_window, GLFW_TRUE);
	}
}