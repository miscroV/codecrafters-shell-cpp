#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>

std::string strip(std::string str);
std::string lstrip(std::string str);
int handle_native_commands(std::string command, std::vector<std::string> args);
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
    // convert line to stream, allows for easier use.
    std::istringstream line_stream(input);

    // read command from stream
    line_stream >> command;

    // pack arguments to vector :
    std::vector<std::string> args;
    args.reserve(16); // preallocate space for 16 arguments
    while (line_stream >> input) {
      args.push_back(input); // add argument to vector
    }

    // handle empty / native / other commands
    if (command.empty()) {
    }
    else if (std::binary_search(native_commands.begin(), native_commands.end(), command)) {
      err_code = handle_native_commands(command, args);
    } 
    else {
      err_code = handle_commands(command, args);
    }

    if (err_code == 0) {break;}
  }
}

int handle_native_commands(std::string command, std::vector<std::string> args) {
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
  else if (command == "echo") {
    for (const auto& arg : args) {
      std::cout << arg << " ";
    }
    std::cout << std::endl;
    return 1;
  }
  return 1;
}
  
int handle_commands(std::string command, std::vector<std::string> args) {
    if (true) {
      std::cout << command << ": command not found" << std::endl;
      return 2;
    }
}
