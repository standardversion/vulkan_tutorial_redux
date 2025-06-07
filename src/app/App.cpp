#include "app/App.h"
#include <GLFW/glfw3.h>
#include <stdexcept>

App::App(uint32_t w, uint32_t h, std::string m_title)
	: m_width{ w }, m_height{ h }, m_title{ m_title }, m_instance{ "Vulkan App", 0, 1, 0 }
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
	m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);
	if (!m_window)
	{
		throw std::runtime_error("Failed to create GLFW Window!");
	}

	m_instance.init(m_window);
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

uint32_t App::get_width() const noexcept
{
	return m_width;
}

uint32_t App::get_height() const noexcept
{
	return m_height;
}

void App::key_callback(GLFWwindow* m_window, int key, int scancode, int action, int mods)
{
	// If ESC is pressed set the close flag to true on the window
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_window, GLFW_TRUE);
	}
}