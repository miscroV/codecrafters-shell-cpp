/*******************************************************************************
 * SHELL HANDLERS IMPLEMENTATION
*******************************************************************************/

#include <vector>
#include <string>
#include <boost/process.hpp>

#include "shell-functions.h"

// COMMAND HANDLER FUNCTION ----------------------------------------------------

int handle_input() {
    std:cout << "TODO" << std::endl;
}

int handle_builtin_errors(int error_code) {
    std:cout << "TODO" << std::endl;
}

int handle_commands(std::string command, std::vector<std::string> args) {
  // list of default commands that handle_commands handles natively. 
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