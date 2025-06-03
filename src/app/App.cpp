#include "app/App.h"
#include <GLFW/glfw3.h>
#include <stdexcept>

App::App(uint32_t w, uint32_t h, std::string m_title)
	: m_width{ w }, m_height{ h }, m_title{ m_title }
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
	m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);
	if (!m_window)
	{
		throw std::runtime_error("Failed to create GLFW Window!");
	}
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

void App::cleanup()
{
	if (m_window) glfwDestroyWindow(m_window);
	glfwTerminate();
}

uint32_t App::get_width() const
{
	return m_width;
}

uint32_t App::get_height() const
{
	return m_height;
}

void App::key_callback(GLFWwindow* m_window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_window, GLFW_TRUE);
	}
}