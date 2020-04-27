#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>


class CounterLock
{
	public:
	CounterLock() { counter = 0; }
	void setNotifyCounterValue(int notifyCounterValue) { this->notifyCounterValue = notifyCounterValue; }
	void lock() { dataLock = std::unique_lock<std::mutex>(mutex); }
	void wait() { cvar.wait(dataLock, [this]{ return counter == notifyCounterValue; }); }
	void unlock() { dataLock.unlock(); }
	void incrementCounter() { counter++; }
	void notifyAll() { cvar.notify_all(); }
	void reset() { counter = 0; }
	private:
	int notifyCounterValue;

	std::unique_lock<std::mutex> dataLock;
	std::atomic<int> counter;
	std::condition_variable cvar;
	std::mutex mutex;
};