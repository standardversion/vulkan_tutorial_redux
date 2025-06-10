#include "app/App.h"
#include "app/Config.h"
#include <iostream>

int main()
{
	try {
		Config cfg{};
		App app{cfg};
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
