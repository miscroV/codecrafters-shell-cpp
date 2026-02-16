/*******************************************************************************
 * SHELL BUILTINS
*******************************************************************************/
#ifndef SHELL_BUILTINS_H
#define SHELL_BUILTINS_H

#include <vector>
#include <string> 

// SHELL BUILTINS --------------------------------------------------------------

/**
 * @brief exit the shell using special err_code (-5)
 */
int exit();

/**
 * @brief echo the following arguments
 * 
 * @param echo_strings
 * @return Int error code returned by the command. 
 * 
 * (0) is success.
 */
int echo(std::vector<std::string> echo_strings);

/**
 * @brief return the type of the argument commands.
 * 
 * @param type_commands Vector(string) containing arguments passed to command. 
 * Normally a list of commands such as 'echo cat dog'
 * @param native_commands Sorted Vector(string) containing list of shell 
 * builtins.
 * @return Int error code returned by the command
 * 
 * (0) is success.
 * (2) no arguments were passed. 
 */
int type(std::vector<std::string> type_commands, 
  std::vector<std::string> native_commands);


/** 
 * @brief return the current working directory 
 * 
 * @return Int error code returned by the command.
 * 
 * (0) is success.
 */
int pwd();


/**
 * @brief change the current working directory
 * 
 * @param args Vector(string) containing arguments passed to command. 
 * Normally a single directory path.
 * @return Int error code returned by the command.  
 */
int cd(std::vector<std::string> args);

#endif // SHELL_BUILTINS