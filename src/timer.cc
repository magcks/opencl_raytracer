#include <cstdlib>
#include <iostream>
#include "color.h"
#include "timer.h"
Timer::Timer() {
	reset();
}
void Timer::reset() {
	start = Timer::now();
}
std::size_t Timer::now() {
	struct timeval now;
	gettimeofday(&now, NULL);
	return (now.tv_sec * 1000000u + now.tv_usec) / 1.e3;
}
std::size_t Timer::get_elapsed() const {
	return Timer::now() - start;
}