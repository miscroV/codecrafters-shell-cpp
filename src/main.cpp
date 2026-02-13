/*******************************************************************************
 * BUILD YOUR OWN SHELL - CODECRAFTERS.IO C++ PROJECT
 * MAIN PROGRAM ENTRY POINT
 * AUTHOR: Miscro V (miscrov@outlook.com)
*******************************************************************************/

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <boost/process.hpp>
#include <iomanip>

#include "shell-functions.h"
//#include "shell-handlers.h"

namespace fs = std::filesystem;
namespace bp = boost::process;

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
  std::vector<std::string> args);

// input HANDLER FUNCTION DECLARATION ----------------------------------------

/**
 * @brief Handle input to correct parseable format from a line.  
 * 
 * @param command_ptr Reference to string to place parsed command in.
 * @param args_ptr Reference to vector of strings to place parsed arguments in. 
 * @return 0
 * 
 * The function parses quotes and endline to use within the shell. 
 * 
 */
int handle_input(
  std::string line, 
  std::string* command_ptr, 
  std::vector<std::string>* args_ptr);

// MAIN PROGRAM LOOP -----------------------------------------------------------

/**
 * @brief Main program loop entry point.
 * 
 * @return int 
 * 
 * Runs the main REPL loop for the shell.
 * print prompt, get user input, parse command and arguments,
 * handle commands via handle_commands function.
 */
int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  bool DEBUG = false;

  int err_code = 1;
  std::string prompt = "$ ";
  while (true) {
    std::cout << prompt;

    std::string line;
    std::getline(std::cin, line);
    boost::trim(line);

    std::string command; 
    std::vector<std::string> args;     
    args.reserve(16);                  

    handle_input(line, &command, &args);
    
    // if empty skip else handle command
    try {
      err_code = handle_commands(command, args);
      if (err_code == -5) {break;} // CONVERT TO CATCH EXECPTION (ExitShell exception and clean exist)
    } 
    catch (const std::exception& e) {
      if (DEBUG != true) {
        std::cout << "Exception: \n" << e.what();
        std::cout << std::endl;
      } 
      else {
        throw;
      }
    }
  }
}


// COMMAND HANDLER FUNCTION ----------------------------------------------------

int handle_input(
  std::string line, 
  std::string* command_ptr, 
  std::vector<std::string>* args_ptr)
  {
  std::string nextArg = "";
  bool dquoted = false;
  bool squoted = false;
  bool isArg = false;
  for (std::string::iterator ch = line.begin(); ch !=line.end(); ++ch) {
    if (*ch == '\"') {
      dquoted = !dquoted;
      if (std::next(ch) == line.end()) {isArg = true;}
    }
    else if (*ch == '\'' && !dquoted) {
      squoted = !squoted;
      if (std::next(ch) == line.end()) {isArg = true;}
    } 
    else if (squoted || dquoted) {
      nextArg += *ch;
      isArg = false;
    }
    else if (std::next(ch) == line.end()) {
      nextArg += *ch;
      isArg = true;
    }  
    else if (!isspace(*ch)) {
      nextArg += *ch;
      isArg = false;
    }
    else {
      isArg = true;
    }

    if (!isArg || nextArg.length() == 0) {continue;}

    // ADD ARGUMENTS ---------------------#
    if (command_ptr->length() == 0) {
      *command_ptr = nextArg;
    }
    else {
      args_ptr->push_back(nextArg);
    }
    nextArg.clear();
    isArg = false;
  }
  return 0;
}

int handle_commands(std::string command, std::vector<std::string> args) {
  // list of default commands that handle_commands handles natively. 

  if (command.empty()) {
    return 0;
  }

  std::vector<std::string> native_commands = {
    "exit", "echo", "type", // Basic Commands
    "pwd", "cd"             // Navigation Commands
  };
  std::sort(native_commands.begin(), native_commands.end());

  // native commands switch
  if      (command == "exit") { return exit(); }
  else if (command == "echo") { return echo(args); }
  else if (command == "type") { return type(args, native_commands); }
  else if (command == "cd"  ) { return cd(args); }
  else if (command == "pwd" ) { return pwd(); }
  // default to finding command in path and run child process
  else if (fs::path execpath = get_executable_path(command); 
    !execpath.empty()) {
    args.insert(args.begin(), command);  
    bp::child c(
      bp::args(args),
      bp::std_out > stdout,
      bp::std_err > stderr,
      bp::std_in  < stdin
      );
    c.wait();
    return c.exit_code();
  }
  // if command not found:
  else {
    std::cout << command << ": command not found" << std::endl;
  return 0;
  }
}   
