#ifndef _CMDLINE_H
#define _CMDLINE_H

#include <selforg/configurable.h> 
#include <vector>

using ConfigList = std::vector<Configurable*>; 

std::vector<std::string> splitString(const std::string& str, char seperator);

/// Shows the values of all parameters of the given configurable objects.
// @param file print to file or stdout if nullptr
// @param lineprefix is used as prefix for each line if not nullptr
void showParams(const ConfigList& configs, FILE* file = nullptr, const char* lineprefix = nullptr);

bool control_c_pressed();
void cmd_handler_init();
void cmd_end_input();

#endif
