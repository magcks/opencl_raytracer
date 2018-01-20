#include "info.h"
std::string Info::str() {
	std::size_t resize = getBiggestNameLength() + extraSpace + getChildrenDepth() * spaceWidth;
	std::stringstream ss;
	std::string spaces = "";
	spaces.resize(spaceWidth, ' ');
	ss << starColor << "*** " << titleColor << title << starColor << " ***" << Color::reset << std::endl;
	ss << headingColor << getTitleLine(8) << Color::reset << std::endl;
	for (std::size_t i = 0; i < attributes.size(); ++i) {
		std::stringstream nameSs;
		nameSs << leftSideColor << attributes[i].first << Color::reset << ":";
		std::string name = nameSs.str();
		std::string val = attributes[i].second;
		name.resize(resize, '.');
		if (maxValueSize > 0 && val.size() > maxValueSize) {
			val = val.substr(0, maxValueSize - 3) + "…";
		}
		ss << name << rightSideColor << val << Color::reset << std::endl;
	}
	if (children.size() > 0)
		ss << std::endl;
	for (std::size_t i = 0; i < children.size(); ++i) {
		std::string childStr = children[i].str();
		ss << spaces << childStr.substr(0, childStr.size() - spaceWidth) << std::endl;
	}
	return ss.str();
}
std::size_t Info::getBiggestNameLength() {
	std::size_t max = 0;
	for (std::size_t i = 0; i < attributes.size(); ++i)
		max = std::max(max, attributes[i].first.size());
	for (std::size_t i = 0; i < children.size(); ++i)
		max = std::max(max, children[i].getBiggestNameLength());
	return max;
}
std::size_t Info::getChildrenDepth() {
	std::size_t depth = 0;
	for (std::size_t i = 0; i < children.size(); ++i)
		depth = std::max(depth, children[i].getChildrenDepth());
	return depth + 1;
}
std::string Info::getTitleLine(std::size_t extra) {
	std::string heading;
	heading.resize(title.size() + extra, '=');
	return heading;
}