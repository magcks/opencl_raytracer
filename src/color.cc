#include "color.h"
#ifndef NO_COLORS
const char *Color::RESET = "\033[0m";
const char *Color::RED = "\033[0;91m";
const char *Color::GREEN = "\033[0;92m";
const char *Color::YELLOW = "\033[0;93m";
const char *Color::BLUE = "\033[0;94m";
const char *Color::PURPLE = "\033[0;95m";
const char *Color::CYAN = "\033[0;96m";
const char *Color::WHITE = "\033[0;97m";
#else
const char *Color::RESET = "";
const char *Color::RED = "";
const char *Color::GREEN = "";
const char *Color::YELLOW = "";
const char *Color::BLUE = "";
const char *Color::PURPLE = "";
const char *Color::CYAN = "";
const char *Color::WHITE = "";
#endif