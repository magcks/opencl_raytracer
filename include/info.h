#pragma once
#include <functional>
#include <sstream>
#include <string>
#include <vector>
#include "color.h"
class Info {
	public:
		Info(std::size_t maxValueSize, std::size_t extraSpace) : maxValueSize(maxValueSize), extraSpace(extraSpace) {
		}
		Info() : maxValueSize(100), extraSpace(3) {
		}
		static std::size_t measure(const std::string &jobDescription, const std::function<bool ()> &job, bool synchronous = false);
		static std::string formatTime(const std::size_t elapsed);
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
		template <class T>
		static std::string toString(const T &val) {
			std::stringstream ss;
			ss << val;
			return ss.str();
		}
		/* TODO: Should probably be private */
		struct Color {
			static const std::string LEFT;
			static const std::string RIGHT;
			static const std::string STAR;
			static const std::string TITLE;
			static const std::string HEADING;
			static const std::string COLON;
			static const std::string PERIOD;
			static const std::string NORMAL;
			static const std::string HIGHLIGHT;
			static const std::string SECTION;
			static const std::string WARNING;
		};
	private:
		const std::size_t maxValueSize;
		const std::size_t extraSpace;
		std::string title;
		std::vector<std::pair<std::string, std::string>> attributes;
		std::vector<Info> children;
		std::string getTitleLine(std::size_t extra) const;
		static std::string repeat(const std::string &word, std::size_t times) {
			std::string result;
			result.reserve(word.length() * times);
			for (auto i = 0u ; i < times ; ++i) {
				result += word;
			}
			return result;
		}
};