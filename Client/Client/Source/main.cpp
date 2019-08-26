#include "Interface.h"

#include <memory>

int main()
{
	std::unique_ptr<Interface> userInterface(std::make_unique<Interface>());
	userInterface->run();
	system("pause");
	return 0;
}