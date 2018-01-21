#pragma once
#include <sstream>
#include <string>
#include <cmath>
static const char *DEF = "-D";
static char ENDDEF = ' ';
struct CompilerOptions {
		inline void add(const char *name, bool v) {
			if (!v)
				return;
			ss << DEF << name << ENDDEF;
		}
		inline void add(const char *name, float v) {
			ss << DEF << name << '=' << v;
			if (v == std::floor(v)) {
				ss << '.';
			}
			ss << 'f' << ENDDEF;
		}
		template <typename T>
		void add(const char *name, T v) {
			
			ss << DEF << name << '=' << v << ENDDEF;
		}
		inline std::string str() {
			return ss.str();
		}
	private:
		std::stringstream ss;
};