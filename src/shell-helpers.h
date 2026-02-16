/*******************************************************************************
 * SHELL HELPERS
*******************************************************************************/
#ifndef SHELL_HELPERS_H
#define SHELL_HELPERS_H

#include <boost/process.hpp>
#include <filesystem>
#include <string>

// HELPER FUNCTIONS ------------------------------------------------------------

/**
 * @brief Help Function: Gets the full path of an executable if it exists. 
 * 
 * @param command String value of command that will be searched for in $PATH
 * @return std::filesystem::path value of executable or empty std::filesystem::path.
 */
std::filesystem::path get_executable_path(const std::string& command);

/**
 * @brief Helper Function: read line by line to out_pipe from pipe. allows for redirection of std::cout in main. 
 * 
 * @param pipe the pipe that async input is pulled from i.e. the child process. 
 * @param out_pipe the stream that data is pushed to from pipe. 
 */
void read_async(boost::process::async_pipe& pipe, std::ostream& out_pipe);

/**
 * @brief Helper Function: run child process and account for redirect possibilities. 
 * 
 * @param command the command to run. 
 * @param args the arguments that will be passed to the command. 
 * 
 * @param r_stdout 'redirect standard out' true if stdout should be redirected to a pipe. 
 * @param r_stderr 'redirect standard error' true if stderr should be redirected to a pipe. 
 * 
 */
int run_path_command(
  std::string command, 
  std::vector<std::string> args,
  bool r_stdout,
  bool r_stderr);

#endif // SHELL_HELPERS