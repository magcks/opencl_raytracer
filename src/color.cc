#include "color.h"
#ifndef NO_COLORS
const std::string Color::reset       = "\033[0m";
const std::string Color::red         = "\033[0;91m";
const std::string Color::green       = "\033[0;92m";
const std::string Color::yellow      = "\033[0;93m";
const std::string Color::blue        = "\033[0;94m";
const std::string Color::purple      = "\033[0;95m";
const std::string Color::cyan        = "\033[0;96m";
const std::string Color::white       = "\033[0;97m";
#else
const std::string Color::reset       = "";
const std::string Color::red         = "";
const std::string Color::green       = "";
const std::string Color::yellow      = "";
const std::string Color::blue        = "";
const std::string Color::purple      = "";
const std::string Color::cyan        = "";
const std::string Color::white       = "";
#endif