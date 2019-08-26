#include "Interface.h"

#include <memory>

int main(int argc, char** argv)
{
	std::unique_ptr<Interface> userInterface(new Interface);
	userInterface->run();
	system("pause");
	return 0;
}