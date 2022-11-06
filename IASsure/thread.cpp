#include "thread.h"

IASsure::thread::PeriodicAction::PeriodicAction(std::chrono::milliseconds initialDelay, std::chrono::milliseconds delay, std::function<void()> f) :
	shouldStop(false),
	f(std::move(f)),
	initialDelay(initialDelay),
	delay(delay),
	t(&IASsure::thread::PeriodicAction::threadFn, this)
{
}

IASsure::thread::PeriodicAction::~PeriodicAction()
{
	this->stop();
	this->t.join();
}

void IASsure::thread::PeriodicAction::stop()
{
	{
		std::scoped_lock<std::mutex> lock(this->m);
		this->shouldStop = true;
	}
	this->c.notify_one();
}

bool IASsure::thread::PeriodicAction::wait(std::chrono::milliseconds delay)
{
	std::unique_lock<std::mutex> lock(this->m);
	this->c.wait_for(lock, delay, [this]() { return this->shouldStop; });
	return !this->shouldStop;
}

void IASsure::thread::PeriodicAction::threadFn()
{
	for (auto delay = this->initialDelay; this->wait(delay); delay = this->delay) {
		this->f();
	}
}
