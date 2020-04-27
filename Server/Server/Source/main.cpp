#include "Interface.h"
#include "CounterLock.h"

#include <memory>
#include <thread>
#include <vector>
#include <condition_variable>

bool f = false;
std::mutex m;
std::condition_variable cv;

void x()
{
	Sleep(5000);
	f = true;
	std::cout << "DONE" << '\n';
	cv.notify_all();
}

void f1(int i )
{
	std::unique_lock<std::mutex> g(m);
	cv.wait(g, []{ return f; });
	g.unlock();
	std::cout << i << '\n';
}

int main(int argc, char** argv)
{
	std::unique_ptr<Interface> userInterface(new Interface);
	userInterface->run();
	system("pause");
	return 0;
}