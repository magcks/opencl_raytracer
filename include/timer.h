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