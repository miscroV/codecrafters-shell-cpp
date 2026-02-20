/*******************************************************************************
 * SHELL HANDLERS
*******************************************************************************/
#ifndef SHELL_HANDLERS_H
#define SHELL_HANDLERS_H

#include <deque>
#include <string>
#include <vector>
#include <fstream>
// SHELL FUNCTIONS -------------------------------------------------------------

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
int handle_commands(
  std::string command, 
  std::vector<std::string> args,
  bool r_stdout,
  bool r_stderr);

/**
 * @brief Gather input from the user, then tokenize it and add all tokens to tokens_que
 * 
 * @param tokens_que a reference to the deque to place tokens into.
 * @return 0
 * 
 */
int tokenize_input(std::deque<std::string>* tokens_que, std::string line, int flags);
int tokenize_input(std::deque<std::string>* tokens_que, std::string line);

int tokenize_input_loop(std::deque<std::string>* tokens);

/**
 * @brief Extract the next command to run from deque, parsing stream redirects (and pipes?)
 * 
 * @param tokens Pointer to the tokens deque to extract from
 * @param command_ptr location to store the command string
 * @param args_ptr location to pack command arguments to.
 * @param r_stdout location to store file stream (if stdout redirected).
 * @param r_stderr location to store file stream (if stderr redirected)
 */
int extract_next_command(
  std::deque<std::string>* tokens,
  std::string* command_ptr,
  std::vector<std::string>* args_ptr,
  std::ofstream* r_stdout,
  std::ofstream* r_stderr);

#endif // SHELL_HANDLERS