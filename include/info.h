#pragma once
#include <iterator>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
#include "color.h"
#define leftSideColor Color::YELLOW
#define rightSideColor Color::GREEN
#define starColor Color::YELLOW
#define titleColor Color::RED
#define headingColor Color::BLUE
// TODO:
#define colonColor Color::WHITE
#define periodColor Color::BLUE
class Info {
	public:
		Info(std::size_t maxValueSize, std::size_t extraSpace, std::size_t spaceWidth) : maxValueSize(maxValueSize), extraSpace(extraSpace), spaceWidth(spaceWidth) {
		}
		Info() : maxValueSize(100), extraSpace(10), spaceWidth(5) {
		}
		void setTitle(const std::string &newTitle) {
			title = newTitle;
		}
		template <class T>
		void add(const std::string &name, const T &val) {
			attributes.push_back(std::make_pair(name, Info::toString(val)));
		}
		void add(Info &child) {
			children.push_back(child);
		}
		std::string str();
		std::size_t getBiggestNameLength();
		std::size_t getChildrenDepth();
		template <class T>
		static std::string vec2str(const std::vector<T> &v) {
			std::stringstream ss;
			for (auto i = 0u; i < v.size(); ++i) {
				if (i != 0) {
					ss << ", ";
				}
				ss << v[i];
			}
			return ss.str();
		}
		// Source: http://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
		static void replaceAll(std::string &str, const std::string &from, const std::string &to);
		// Source: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
		// Trim from start
		static std::string &ltrim(std::string &s);
		// Trim from end
		static std::string &rtrim(std::string &s);
		// Trim from both ends
		static std::string &trim(std::string &s);
		template <class T>
		static std::string toString(const T &val) {
			std::stringstream ss;
			ss << val;
			return ss.str();
		}
	private:
		const std::size_t maxValueSize;
		const std::size_t extraSpace;
		const std::size_t spaceWidth;
		std::string title;
		std::vector<std::pair<std::string, std::string>> attributes;
		std::vector<Info> children;
		std::string getTitleLine(std::size_t extra) const;
		static std::string repeat(const std::string &word, std::size_t times) {
			std::string result;
			result.reserve(word.length() * times);
			for (std::size_t i = 0u ; i < times ; ++i) {
				result += word;
			}
			return result;
		}
};