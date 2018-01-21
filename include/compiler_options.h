#pragma once
#include <sstream>
#include <string>
#include <vector>
class CompilerOptions {
	public:
		void add(std::string name, bool v) {
			if (!v)
				return;
			store.push_back(name);
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
			store.push_back(ss.str());
		}
		template <typename T>
		void add(std::string name, T v) {
			std::stringstream ss;
			ss << name << "=" << v;
			store.push_back(ss.str());
		}
		std::string str() {
			std::stringstream ss;
			for (auto i = 0u; i < store.size(); ++i) {
				if (i != 0)
					ss << " ";
				ss << "-D " << store[i];
			}
			return ss.str();
		}
	private:
		std::vector<std::string> store;
};