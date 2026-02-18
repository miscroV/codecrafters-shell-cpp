/*******************************************************************************
 * BUILD YOUR OWN SHELL - CODECRAFTERS.IO C++ PROJECT
 * MAIN PROGRAM ENTRY POINT
 * AUTHOR: Miscro V (miscrov@outlook.com)
*******************************************************************************/

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <deque>
#include <fstream>
#include <unistd.h>
#include <termios.h>

// #include "shell-functions.h"
#include "shell-helpers.h"
#include "shell-handlers.h"
#include "shell-builtins.h"

namespace fs = std::filesystem;
namespace bp = boost::process;

struct termios orig_termios;

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


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
  std::string prompt = "$ ";
  enableRawMode(); // allows for immediating parsing. 
  while (true) {
    std::cout << prompt;

    std::deque<std::string> tokens;
    std::string command; 
    std::vector<std::string> args;     
    args.reserve(16);                
    
    std::ofstream r_stdout;
    std::ofstream r_stderr;
    std::streambuf* stdout_buf;
    std::streambuf* stderr_buf;

    // TODO autocomplete
    // 1 detect tab press using curses.h librar
    // tokens and get last token
    // using a trie datastructure, locate/predict the rest of the command
    // write the rest of the command to the line before reading
    // 

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
      if (iscntrl(c)) {
        printf("%d\n", c);
      } else {
        printf("%d ('%c')\n", c, c);
      }
    }

    
  //   tokenize_input(&tokens);
  //   extract_next_command(&tokens, &command, &args, &r_stdout, &r_stderr);

  //   if (r_stdout.is_open()) {
  //     stdout_buf = std::cout.rdbuf();
  //     std::cout.rdbuf(r_stdout.rdbuf());
  //   }
  //   if (r_stderr.is_open()) {
  //     stderr_buf = std::cerr.rdbuf();
  //     std::cerr.rdbuf(r_stderr.rdbuf());
  //   }

  //   try {
  //     err_code = handle_commands(
  //       command, args, r_stdout.is_open(), r_stderr.is_open()
  //     );
  //     if (err_code == -5) {break;}
  //   } 
  //   catch (const std::exception& e) {
  //     if (DEBUG != true) {
  //       std::cout << "Exception: \n" << e.what();
  //       std::cout << std::endl;
  //     } 
  //     else {
  //       throw;
  //     }
  //   }

  //   if (r_stdout.is_open()) {
  //     std::cout.rdbuf(stdout_buf);
  //     r_stdout.close();
  //   }
  //   if (r_stderr.is_open()) {
  //     std::cerr.rdbuf(stderr_buf);
  //     r_stderr.close();
  //   }
  }
}
