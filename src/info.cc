#include <cstdlib>
#include "info.h"
const std::string Info::Color::LEFT = ::Color::WHITE;
const std::string Info::Color::RIGHT = ::Color::GREEN;
const std::string Info::Color::STAR = ::Color::YELLOW;
const std::string Info::Color::TITLE = ::Color::GREEN;
const std::string Info::Color::HEADING = ::Color::BLUE;
std::string Info::str() {
	const std::string headingMarker = repeat("*", 3);
	const std::string headingMarkerPadding = " ";
	const std::size_t resize = getBiggestNameLength() + extraSpace;
	std::stringstream ss;
	ss
		/* Headline */
		<< Color::STAR << headingMarker << headingMarkerPadding
		<< Color::TITLE << title
		<< Color::STAR << headingMarkerPadding << headingMarker
		<< ::Color::RESET
		<< std::endl
		/* Underline */
		<< Color::HEADING << getTitleLine(2 * (headingMarker.size() + headingMarkerPadding.size()))
		<< ::Color::RESET
		<< std::endl;
	for (auto i = 0u; i < attributes.size(); ++i) {
		std::stringstream nameSs;
		nameSs << Color::LEFT << attributes[i].first << ::Color::RESET;
		std::string name = nameSs.str();
		std::string val = attributes[i].second;
		name.resize(resize + Color::LEFT.size() + ::Color::RESET.size(), '.');
		if (maxValueSize > 0 && val.size() > maxValueSize) {
			val = val.substr(0, maxValueSize - 1) + "…";
		}
		ss << name << Color::RIGHT << val << ::Color::RESET << std::endl;
	}
	if (children.size() > 0)
		ss << std::endl;
	for (auto i = 0u; i < children.size(); ++i) {
		const std::string &childStr = children[i].str();
		ss << childStr.substr(0, childStr.size()) << std::endl;
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
	for (auto i = 0u; i < children.size(); ++i)
		depth = std::max(depth, children[i].getChildrenDepth());
	return depth + 1;
}
std::string Info::getTitleLine(std::size_t extra) const {
	std::string heading;
	heading.resize(title.size() + extra, '=');
	return heading;
}