#include "app/App.h"
#include <iostream>

int main()
{
	try {
		App app{ 600, 400, "Vulkan" };
		app.init();
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
