#include "color.h"
#ifndef NO_COLORS
const std::string Color::RESET = "\033[0m";
const std::string Color::RED = "\033[0;91m";
const std::string Color::GREEN = "\033[0;92m";
const std::string Color::YELLOW = "\033[0;93m";
const std::string Color::BLUE = "\033[0;94m";
const std::string Color::PURPLE = "\033[0;95m";
const std::string Color::CYAN = "\033[0;96m";
const std::string Color::WHITE = "\033[0;97m";
#else
const std::string Color::RESET;
const std::string Color::RED;
const std::string Color::GREEN;
const std::string Color::YELLOW;
const std::string Color::BLUE;
const std::string Color::PURPLE;
const std::string Color::CYAN;
const std::string Color::WHITE;
#endif