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
		Info(std::size_t _maxValueSize, std::size_t _extraSpace, std::size_t _spaceWidth) : maxValueSize(_maxValueSize), extraSpace(_extraSpace), spaceWidth(_spaceWidth) {
		}
		Info() : maxValueSize(100), extraSpace(10), spaceWidth(5) {
		}
		void setTitle(const std::string &newTitle) {
			title = newTitle;
		}
		template <class T>
		void add(const std::string name, const T &val) {
			attributes.push_back(std::make_pair(name, Info::toStr(val)));
		}
		void add(Info child) {
			children.push_back(child);
		}
		std::string str();
		std::size_t getBiggestNameLength();
		std::size_t getChildrenDepth();
		template <class T>
		static std::string vec2str(const std::vector<T> &v) {
			std::stringstream ss;
			for (size_t i = 0; i < v.size(); ++i) {
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
		static std::string toStr(T &val) {
			std::stringstream ss;
			ss << val;
			return ss.str();
		}
	private:
		std::size_t maxValueSize;
		std::size_t extraSpace;
		std::size_t spaceWidth;
		std::string title;
		std::vector<std::pair <std::string, std::string> > attributes;
		std::vector<Info> children;
		std::string getTitleLine(std::size_t extra);
};