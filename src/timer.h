#pragma once
#include <ctime>
#include <sys/time.h>
// Simple CPU clock timer.
class Timer {
	public:
		Timer(void);
		void reset(void);
		static std::size_t now(void);
		std::size_t get_elapsed(void) const;
	private:
		std::size_t start;
};
inline Timer::Timer(void) {
	this->reset();
}
inline void Timer::reset(void) {
	this->start = Timer::now();
}
inline std::size_t Timer::now(void) {
	struct timeval now;
	gettimeofday(&now, NULL);
	return (now.tv_sec * 1000000u + now.tv_usec) / 1.e3;
//	return static_cast<std::size_t>(std::clock()) * 1000
//         / static_cast<std::size_t>(CLOCKS_PER_SEC);
}
inline std::size_t Timer::get_elapsed(void) const {
	return Timer::now() - start;
}