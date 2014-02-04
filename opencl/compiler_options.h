#ifndef COMPILER_OPTIONS_H
#define COMPILER_OPTIONS_H

#include <vector>
#include <sstream>
#include <string>

class CompilerOptions {
public:
	void add(std::string name, bool v) {
		if (!v)
			return;

		this->store.push_back(name);
	}

	void add(std::string name, float v) {
		int rounded = v;

		std::stringstream ssRounded;
		ssRounded << rounded;

		std::stringstream ssV;
		ssV << v;

		std::stringstream ss;
		ss << name << "=" << v;
		if (ssRounded.str() == ssV.str())
			ss << ".0";
		ss << "f";
		this->store.push_back(ss.str());
	}

	template <typename T>
	void add(std::string name, T v) {
		std::stringstream ss;
		ss << name << "=" << v;
		this->store.push_back(ss.str());
	}

	std::string str() {
		std::stringstream ss;
		for (std::size_t i = 0; i < this->store.size(); ++i) {
			if (i != 0)
				ss << " ";

			ss << "-D " << this->store[i];
		}
		return ss.str();
	}
private:
	std::vector<std::string> store;
};

#endif