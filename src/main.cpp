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
  while (true) {
    std::string prompt = "$ ";
    std::cout << prompt;


    std::string command; // empty the command string every loop. 
    std::string input;

    std::getline(std::cin, input);
    std::istringstream line_stream(input); // create stream from input line
    line_stream >> command;                // read command from stream

    std::vector<std::string> args;     // vector to hold arguments
    args.reserve(16);                  // preallocate space for 16 arguments

    while (line_stream >> input) {     // pack arguments to vector :
      args.push_back(input); 
      std::cout << "DEBUG: arg added: " << input << std::endl;
    }

    // if empty skip else handle command
    try {
      if (command.empty()) { /* Do nothing */ } 
      else { err_code = handle_commands(command, args); }

      if (err_code == -5) {break;} // break on exit err_code
    } 
    catch (const std::exception &e) {
      if (!(DEBUG)) {
        std::cout << "Exception\n" e.what();
        std::cout << std::endl;
      } else {
        throw &e
      }

    }
  }
}


