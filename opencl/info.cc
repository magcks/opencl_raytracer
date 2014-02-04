#include "info.h"

std::string Info::str() {
	std::size_t resize = this->getBiggestNameLength() + this->extraSpace + this->getChildrenDepth() * this->spaceWidth;
	std::stringstream ss;

	std::string spaces = "";
	spaces.resize(this->spaceWidth, ' ');

	ss << starColor << "*** " << titleColor << this->title << starColor << " ***" << Color::reset << std::endl;
	ss << headingColor << this->getTitleLine(8) << Color::reset << std::endl;

	for (std::size_t i = 0; i < this->attributes.size(); ++i) {

		std::stringstream nameSs;
		nameSs << leftSideColor << this->attributes[i].first << Color::reset << ":";
		std::string name = nameSs.str();
		std::string val = this->attributes[i].second;

		name.resize(resize, this->ch);

		Info::trim(val);

		if (this->maxValueSize > 0 && val.size() > this->maxValueSize)
			val = val.substr(0, this->maxValueSize - 3) + "...";

		ss << name << rightSideColor << val << Color::reset << std::endl;
	}

	if (this->children.size() > 0)
		ss << std::endl;

	for (std::size_t i = 0; i < this->children.size(); ++i) {
		std::string childStr = this->children[i].str();
		std::stringstream spacesSs;
		spacesSs << "\n" << spaces;
		Info::replaceAll(childStr, "\n", spacesSs.str());
		ss << spaces << childStr.substr(0, childStr.size() - this->spaceWidth) << std::endl;
	}

	return ss.str();
}

std::size_t Info::getBiggestNameLength() {
	std::size_t max = 0;

	for (std::size_t i = 0; i < this->attributes.size(); ++i)
		max = std::max(max, this->attributes[i].first.size());

	for (std::size_t i = 0; i < this->children.size(); ++i)
		max = std::max(max, this->children[i].getBiggestNameLength());

	return max;
}

std::size_t Info::getChildrenDepth() {
	std::size_t depth = 0;

	for (std::size_t i = 0; i < this->children.size(); ++i)
		depth = std::max(depth, this->children[i].getChildrenDepth());

	return depth + 1;
}


// Source: http://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
void Info::replaceAll(std::string &str, const std::string &from, const std::string &to) {
	if (from.empty())
		return;

	size_t start_pos = 0;

	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

// Source: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
// Trim from start
std::string & Info::ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// Trim from end
std::string & Info::rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// Trim from both ends
std::string & Info::trim(std::string &s) {
	return Info::ltrim(Info::rtrim(s));
}


std::string Info::getTitleLine(std::size_t extra) {
	std::string heading;
	heading.resize(this->title.size() + extra, '=');
	return heading;
}