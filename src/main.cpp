#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <filesystem>

std::string strip(std::string str);
std::string lstrip(std::string str);
int handle_native_commands(std::string command, 
    std::vector<std::string> args,
    std::vector<std::string> native_commands
  );
int handle_commands(std::string command, std::vector<std::string> args);

int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::vector<std::string> native_commands = {"exit", "echo", "type"};
  std::sort(native_commands.begin(), native_commands.end());

  int err_code = 1;
  std::string command;

  while (true) {
    
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
    if (command.empty()) {
    }
    else if (std::binary_search(native_commands.begin(), 
      native_commands.end(), command) ) {
      err_code = handle_native_commands(command, args, native_commands);
    } 
    else {
      err_code = handle_commands(command, args);
    }

    if (err_code == 0) {break;}
  }
}

int handle_native_commands(std::string command, 
    std::vector<std::string> args,
    std::vector<std::string> native_commands
  ) {
  if (command == "exit") {
    return 0; // Exit the shell
  }
  else if (command == "echo") {
    for (const auto& arg : args) {
      std::cout << arg << " ";
    }
    std::cout << std::endl;
    return 1;
  }
  else if (command == "type") {
    if (args.empty()) {
      return 2;
    }

    for (const auto& arg : args) {
      if (std::binary_search(native_commands.begin(), 
      native_commands.end(), arg) ) {
        std::cout << arg << " is a shell builtin" << std::endl;
      } 
      else {
        // Get Path Variable as dir strings. 
        std::string PATH = std::getenv("PATH");
        std::istringstream path_stream(PATH);
        std::string dir;
        // Iterate through PATH directories and locate the command
        while (std::getline(path_stream, dir, ':')) {
          std::filesystem::path full_path = std::filesystem::path(dir) / arg;
          // Check if the file exists, is a regular file, and is executable
          bool is_executable = !(
            std::filesystem::exists(full_path) && 
            std::filesystem::is_regular_file(full_path) && (
            (std::filesystem::status(full_path).permissions() & 
            std::filesystem::perms::owner_exec) != std::filesystem::perms::none ||
            (std::filesystem::status(full_path).permissions() & 
            std::filesystem::perms::group_exec) != std::filesystem::perms::none ||
            (std::filesystem::status(full_path).permissions() & 
            std::filesystem::perms::others_exec) != std::filesystem::perms::none
            ));

          if (is_executable) {
            std::cout << arg << " is " << full_path << std::endl;
            return 1;
          }
          else {
            std::cout << arg << ": not found" << std::endl;
            return 1;
          }
      }
    } 
    return 1;
   }
  }
  std::cout << command << ": native command not handled" << std::endl;
  return 1;
}

int handle_commands(std::string command, std::vector<std::string> args) {
    if (true) {
      std::cout << command << ": command not found" << std::endl;
      return 2;
    }
}
