#include <cstdint>
#include <string>

class GLFWwindow;

class App
{
public:
	App(uint32_t w, uint32_t h, std::string m_title);
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	~App();
	void init();
	void run();
	uint32_t get_width() const;
	uint32_t get_height() const;

	static void key_callback(GLFWwindow* m_window, int key, int scancode, int action, int mods);

private:
	uint32_t m_width;
	uint32_t m_height;
	std::string m_title;
	GLFWwindow* m_window;

	void cleanup();
};