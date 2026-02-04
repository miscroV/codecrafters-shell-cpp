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
#include <iomanip>
#include <stdexcept>

#include "shell-functions.h"
#include "shell-handlers.h"

namespace fs = std::filesystem;
namespace bp = boost::process;

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
    std::string command; // empty the command string every loop. 
    std::string nextArg = "";

    std::getline(std::cin, line);

    std::vector<std::string> args;     // vector to hold arguments
    args.reserve(16);                  // preallocate space for 16 arguments

    bool quoted = false;
    for (std::string::iterator ch = line.begin(); ch !=line.end(); ++ch) {
      if (*ch == '\'' || *ch == '\"') {
        if (DEBUG) {std::cout << "Quoted Toggled";}
        quoted = !quoted;
        continue;
      }
      if (quoted) {
        nextArg += *ch;
        if (DEBUG) {std::cout << *ch << " added to next arg"}
        continue;
      }

      if (isspace(*ch)) {
        continue;
      }
      nextArg += *ch;
    }

    // if empty skip else handle command
    try {
      if (command.empty()) { /* Do nothing */ } 
      else { err_code = handle_commands(command, args); }
      if (err_code == -5) {break;} // CONVERT TO CATCH EXECPTION (ExitShell exception and clean exist)
    } 
    catch (const std::exception& e) {
      if (DEBUG != true) {
        std::cout << "Exception: \n" << e.what();
        std::cout << std::endl;
      } else {
        throw;
    }
  }
}


