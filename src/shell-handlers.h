/*******************************************************************************
 * SHELL HANDLERS HEADER
*******************************************************************************/

#ifndef SHELL_HANDLERS_H
#define SHELL_HANDLERS_H

#include <string>
#include <vector>


// SHELL HANDLERS --------------------------------------------------------------

// COMMAND HANDLER FUNCTION DECLARATION ----------------------------------------

/**
 * @brief Command Switch / Handler Function
 * 
 * @param command String value of command to be ran.
 * @param args Vector array of arguments to send to the command
 * @return Int error code returned by the command. 
 * 
 * @note Native commands are {exit, echo, type}
 * 
 */
int handle_commands(std::string command, std::vector<std::string> args);


#endif