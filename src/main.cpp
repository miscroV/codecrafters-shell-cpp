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

int handle_commands(std::string command, std::vector<std::string> args);
fs::path get_executable_path(const std::string& command);

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

int handle_commands(std::string command, std::vector<std::string> args) {
  std::vector<std::string> native_commands = {
    "exit", "echo", "type"
  };
  std::sort(native_commands.begin(), native_commands.end());

  // Execute exit command
  if (command == "exit") {
    return -5; 
  }
  // Execute echo command
  else if (command == "echo") {
    for (const auto& arg : args) {
      std::cout << arg << " ";
    }
    std::cout << std::endl;
    return 0;
  }
  // Execute type command
  else if (command == "type") {
  
    if (args.empty()) {
      return 2;
    }

    for (const auto& arg : args) {
      if (std::binary_search(native_commands.begin(), 
      native_commands.end(), arg) ) {
        std::cout << arg << " is a shell builtin" << std::endl;
      } 
      else if (fs::path argpath = get_executable_path(arg); !argpath.empty()) {
        std::cout << arg << " is " << argpath.generic_string() << std::endl;
      }
      else {
        std::cout << arg << ": not found" << std::endl;
      } 
    }
    return 0;
  }
  // Execute external command
  else if (fs::path execpath = get_executable_path(command); !execpath.empty()) {
    args.insert(args.begin(), command);  
    bp::child c(
          // execpath.generic_string(),
          bp::args(args),
          bp::std_out > stdout,
          bp::std_err > stderr
        );
      c.wait();
      return c.exit_code();
    // std::cout << execpath.filename().generic_string() << ": launched with PID " << c.id() << std::endl;

  }
  // Command not found
  else {
  std::cout << command << ": command not found" << std::endl;
  return 0;
  }
}

fs::path get_executable_path(const std::string& command) {
  std::string PATH = std::getenv("PATH");
  std::istringstream path_stream(PATH);
  std::string dir;

  while (std::getline(path_stream, dir, ':')) {
    fs::path full_path = fs::path(dir) / command;
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


