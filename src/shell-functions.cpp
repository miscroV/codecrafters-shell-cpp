/*******************************************************************************
 * SHELL BUILTIN FUNCTIONS IMPLEMENTATION
*******************************************************************************/

#include "shell-functions.h"

#include <iostream>
#include <filesystem>
#include <algorithm>

// SHELL BUILTINS --------------------------------------------------------------

int exit(void) {
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
    else if (std::filesystem::path execpath = get_executable_path(command); 
      !execpath.empty()) {
      std::cout << command << " is " << execpath.generic_string() << std::endl;
    }
    else {
      std::cout << command << ": not found" << std::endl;
    } 
  }
  return 0; //return success
}

int pwd() {
  std::cout << std::filesystem::current_path().generic_string() << std::endl;
  return 0; //return success
}

int cd(std::vector<std::string> args) {
  if (args.empty() || args[0] == "~") {
    std::filesystem::path home_path = std::getenv("HOME");
    std::filesystem::current_path(home_path);
  }
  else {
    std::filesystem::path target_path = args[0];
    std::error_code ec;
    std::filesystem::current_path(target_path, ec);
    if (ec) {
      std::cout << "cd: " << target_path.generic_string() 
        << ": " << ec.message() << std::endl;
      return 1;
    }
  }
  return 0;
  } 
// HELPER FUNCTIONS ------------------------------------------------------------

std::filesystem::path get_executable_path(const std::string& command) {
  std::string PATH = std::getenv("PATH");
  std::istringstream path_stream(PATH);
  std::string dir;

  while (std::getline(path_stream, dir, ':')) {
    std::filesystem::path full_path = std::filesystem::path(dir) / command;

    // check if file exists, is a regular file, 
    // and is executable by current user. 
    bool is_executable = (
    std::filesystem::exists(full_path) && 
    std::filesystem::is_regular_file(full_path) && (
    (std::filesystem::status(full_path).permissions() 
      & std::filesystem::perms::owner_exec) 
      != std::filesystem::perms::none ||
    (std::filesystem::status(full_path).permissions() 
      & std::filesystem::perms::group_exec) 
      != std::filesystem::perms::none ||
    (std::filesystem::status(full_path).permissions() 
      & std::filesystem::perms::others_exec) 
      != std::filesystem::perms::none
      )
    );
    
    if (is_executable) {
      return full_path;
      break;
    }
  }
  return std::filesystem::path();
}
