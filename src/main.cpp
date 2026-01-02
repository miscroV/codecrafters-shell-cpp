#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>

std::string strip(std::string str);
std::string lstrip(std::string str);
int handle_native_commands(std::string command);
int handle_commands(std::string command);


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

    line_stream >> command;

    err_code = handle_native_commands(command);
    if (err_code==0) {break;} else if (err_code==1) {continue;} // check status
    err_code = handle_commands(command);
  }
}


int handle_native_commands(std::string command) {
  if ( strip(command)=="exit" ) {
    return 0; // Exit the shell
  } 
  else if ( lstrip(command).substr(0, 4)=="echo") {
    std::string to_echo = lstrip(command).substr(4); // Get the rest of the command after "echo"
    std::cout << to_echo << std::endl; // Print the echoed string
    return 1;
  } else if ( lstrip(command).substr(0, 4)=="type") {
    std::string command = strip(lstrip(command).substr(4));

    return 1;
  } 
  else if (lstrip(command).empty()) {
    return 1;
  } 
  return 2; //continue running
}


int handle_commands(std::string command) {
    if (true) {
      std::cout << command << ": command not found" << std::endl;
      return 2;
    }
}
