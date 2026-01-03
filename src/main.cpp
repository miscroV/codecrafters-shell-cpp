#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <boost/process.hpp>

namespace fs = std::filesystem;
namespace bp = boost::process;

/*******************************************************************************
 * DOCUMENTATION
*******************************************************************************/

/**
 * @brief the command workhorse running both shell builtins and externals
 * 
 * @param command String value of command to be ran.
 * @param args Vector array of arguments to send to the command
 * @return Int error code returned by the command. 
 * 
 * @note Native commands are {exit, echo, type}
 * 
 */
int handle_commands(std::string command, std::vector<std::string> args);

// SHELL BUILTINS -------------------------------------------------------------

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
 * (0) is success
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

// HELPER FUNCTION -------------------------------------------------------------

/**
 * @brief Help Function: Gets the full path of an executable if it exists. 
 * 
 * @param command String value of command that will be searched for in $PATH
 * @return std::filesystem::path value of executable or empty fs::path.
 */
fs::path get_executable_path(const std::string& command);


// BEGIN MAIN CODE -------------------------------------------------------------

/*******************************************************************************
 * MAIN REPL LOOP
*******************************************************************************/
int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  int err_code = 1;
  while (true) {
    std::string command; // empty the command string every loop. 
    std::cout << "$ ";
    std::string input;
    std::getline(std::cin, input);
    
    std::istringstream line_stream(input); // create stream from input line
    line_stream >> command;                // read command from stream

    std::vector<std::string> args;     // vector to hold arguments
    args.reserve(16);                  // preallocate space for 16 arguments
    while (line_stream >> input) {     // pack arguments to vector :
      args.push_back(input); 
    }
    // handle empty / native / other commands
    if (command.empty()) {} 
    else {
      err_code = handle_commands(command, args);
    }
    if (err_code == -5) {break;}
  }
}
/*******************************************************************************
 * PRIMARY COMMAND HANDLING
*******************************************************************************/
int handle_commands(std::string command, std::vector<std::string> args) {
  // list of default commands that handle_commands handles natively. 
  std::vector<std::string> native_commands = {
    "exit", "echo", "type"
  };
  std::sort(native_commands.begin(), native_commands.end());

  // native commands switch
  if      (command == "exit") { return exit(); }
  else if (command == "echo") { return echo(args); }
  else if (command == "type") { return type(args, native_commands); }
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
/*******************************************************************************
 * SHELL BUILTIN FUNCTIONS
*******************************************************************************/
int exit() {
  return -5; //exit code
}

int echo(std::vector<std::string> echo_strings) {
    for (const auto& arg : echo_strings) {
      std::cout << arg << " ";
    }
    std::cout << std::endl;
    return 0; //return success
}

int type(std::vector<std::string> type_commands, 
  std::vector<std::string> native_commands) {
  if (type_commands.empty()) {
      return 2; //ERROR Missing parameters
  }
  for (const auto& command : type_commands) {
    if (std::binary_search(native_commands.begin(), 
      native_commands.end(), command) ) {
      std::cout << command << " is a shell builtin" << std::endl;
    } 
    else if (fs::path execpath = get_executable_path(command); 
      !execpath.empty()) {
      std::cout << command << " is " << execpath.generic_string() << std::endl;
    }
    else {
      std::cout << command << ": not found" << std::endl;
    } 
  }
  return 0; //return success
}

/*******************************************************************************
 * HELPER FUNCTIONS
*******************************************************************************/
fs::path get_executable_path(const std::string& command) {
  std::string PATH = std::getenv("PATH");
  std::istringstream path_stream(PATH);
  std::string dir;

  while (std::getline(path_stream, dir, ':')) {
    fs::path full_path = fs::path(dir) / command;

    // check if file exists, is a regular file, 
    // and is executable by current user. 
    bool is_executable = (
    fs::exists(full_path) && 
    fs::is_regular_file(full_path) && (
    (fs::status(full_path).permissions() & fs::perms::owner_exec) 
      != fs::perms::none ||
    (fs::status(full_path).permissions() & fs::perms::group_exec) 
      != fs::perms::none ||
    (fs::status(full_path).permissions() & fs::perms::others_exec) 
      != fs::perms::none
      )
    );
    
    if (is_executable) {
        return full_path;
        break;
    }
  }
  return fs::path();
}
