#include "info.h"
#include <iostream>
std::string Info::str() {
	const std::string headingMarker = repeat("*", 3);
	const std::string headingMarkerPadding = " ";
	std::size_t resize = getBiggestNameLength() + extraSpace + getChildrenDepth() * spaceWidth;
	std::stringstream ss;
	ss << starColor << headingMarker << headingMarkerPadding << titleColor << title << starColor << headingMarkerPadding << headingMarker << Color::RESET << std::endl;
	ss << headingColor << getTitleLine(2 * (headingMarker.size() + headingMarkerPadding.size())) << Color::RESET << std::endl;
	for (auto i = 0u; i < attributes.size(); ++i) {
		std::stringstream nameSs;
		nameSs << leftSideColor << attributes[i].first << Color::RESET << ":";
		std::string name = nameSs.str();
		std::string val = attributes[i].second;
		name.resize(resize, '.');
		if (maxValueSize > 0 && val.size() > maxValueSize) {
			val = val.substr(0, maxValueSize - 1) + "…";
		}
		ss << name << rightSideColor << val << Color::RESET << std::endl;
	}
	if (children.size() > 0)
		ss << std::endl;
	for (auto i = 0u; i < children.size(); ++i) {
		const std::string &childStr = children[i].str();
		ss << childStr.substr(0, childStr.size() - spaceWidth) << std::endl << std::endl;
	}
	return ss.str();
}
std::size_t Info::getBiggestNameLength() {
	std::size_t max = 0;
	for (auto i = 0u; i < attributes.size(); ++i)
		max = std::max(max, attributes[i].first.size());
	for (auto i = 0u; i < children.size(); ++i)
		max = std::max(max, children[i].getBiggestNameLength());
	return max;
}
std::size_t Info::getChildrenDepth() {
	std::size_t depth = 0;
	for (auto i = 0u; i < children.size(); ++i)
		depth = std::max(depth, children[i].getChildrenDepth());
	return depth + 1;
}
std::string Info::getTitleLine(std::size_t extra) const {
	std::string heading;
	heading.resize(title.size() + extra, '=');
	return heading;
}