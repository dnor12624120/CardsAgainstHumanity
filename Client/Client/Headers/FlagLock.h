#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>


class FlagLock
{
	public:
	FlagLock() :
		flag{ false }
	{ }
	void setFlag(bool flag) { this->flag = flag; }
	void lock() { dataLock = std::unique_lock<std::mutex>(mutex); }
	void wait() { cvar.wait(dataLock, [this]{ return flag; }); }
	void unlock() { dataLock.unlock(); }
	void notifyAll() { cvar.notify_all(); }
	void reset() { flag = false; }
	private:
	bool flag;
	std::unique_lock<std::mutex> dataLock;
	std::condition_variable cvar;
	std::mutex mutex;
};