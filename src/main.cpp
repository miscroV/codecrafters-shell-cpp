#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <filesystem>


namespace fs = std::filesystem;

int handle_commands(std::string command, 
    std::vector<std::string> args,
    std::vector<std::string> native_commands
  );
fs::path get_executable_path(const std::string& command);

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
    else {
      err_code = handle_commands(command, args, native_commands);
    }

    if (err_code == 0) {break;}
  }
}

int handle_commands(std::string command, 
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
      else if (fs::path argpath = get_executable_path(arg); !argpath.empty()) {
        std::cout << arg << " is " << argpath.generic_string() << std::endl;
      }
      else {
        std::cout << arg << ": not found" << std::endl;
      } 
    }
    return 1;
  }
  else if (fs::path execpath = get_executable_path(command); !execpath.empty()) {
    // Execute external command
    args.push_back(nullptr); // Null-terminate the argument list

    pid_t pid = fork();
    if (pid == 0) {
      // Child process
      execv(exec_path.c_str(), const_cast<char* const*>(c_args.data()));
      std::cerr << "Failed to execute " << command << std::endl;
      std::exit(1);
    } 
    else if (pid > 0) {
      // Parent process
      int status;
      waitpid(pid, &status, 0);
      return 1;
    } 
    else {
      std::cerr << "Fork failed" << std::endl;
      return 1;
    }
  }
  std::cout << command << ": command not found" << std::endl;
  return 1;
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


