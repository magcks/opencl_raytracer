#pragma once
#include <ctime>
#include <sys/time.h>
// Simple CPU clock timer.
class Timer {
	public:
		Timer();
		void reset();
		static std::size_t now();
		std::size_t get_elapsed() const;
	private:
		std::size_t start;
};
inline Timer::Timer() {
	reset();
}
inline void Timer::reset() {
	start = Timer::now();
}
inline std::size_t Timer::now() {
	struct timeval now;
	gettimeofday(&now, NULL);
	return (now.tv_sec * 1000000u + now.tv_usec) / 1.e3;
//	return static_cast<std::size_t>(std::clock()) * 1000
//         / static_cast<std::size_t>(CLOCKS_PER_SEC);
}
inline std::size_t Timer::get_elapsed() const {
	return Timer::now() - start;
}