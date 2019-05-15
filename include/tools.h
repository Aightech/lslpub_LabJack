/**
 * \file tools.cpp
 * \brief TODO.
 * \author Alexis Devillard
 * \version 1.0
 * \date 08 may 2019
 */


#ifndef TOOLS_H
#define TOOLS_H
#include <iostream>
#include <pqxx/pqxx>
#include <string>

/**
 * @brief error Display the passed string thne exit the program.
 * @param str String to display.
 */
void error(std::string str);

/**
 * @brief usage Display the usage, then exit the program.
 * @param optf List of option flags
 * @param optf List of option labels
 * @param optf List of option values
 */
void usage(std::vector<std::string>& optf,  std::vector<std::string>& optl, std::vector<std::string>& optv);


/**
 * @brief get_arg Search for the potential argument in the argument passed to the program.
 * @param argc Argument counter
 * @param argv Argument array
 * @param optf List of option flags
 * @param optf List of option labels
 * @param optf List of option values
 */
void get_arg(int argc, char ** argv, std::vector<std::string>& optf,  std::vector<std::string>& optl, std::vector<std::string>& optv);
#endif
