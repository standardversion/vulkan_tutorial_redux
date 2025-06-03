#include "app/App.h"

int main()
{
	App app{ 600, 400, "Vulkan" };
	app.init();
	app.run();
	return 0;
}
