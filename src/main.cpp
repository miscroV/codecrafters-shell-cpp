#include <iostream>
#include <string>

int main() {
  while (true) {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // TODO: Uncomment the code below to pass the first stage
    std::cout << "$ ";

    std::string command;
    std::getline(std::cin, command);

    std::cout << command << ": command not found" << std::endl;
  }
}