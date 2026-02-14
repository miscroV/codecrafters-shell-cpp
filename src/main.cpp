/*******************************************************************************
 * BUILD YOUR OWN SHELL - CODECRAFTERS.IO C++ PROJECT
 * MAIN PROGRAM ENTRY POINT
 * AUTHOR: Miscro V (miscrov@outlook.com)
*******************************************************************************/

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <boost/process.hpp>
#include <iomanip>
#include <deque>
#include <stack>

#include "shell-functions.h"
//#include "shell-handlers.h"

namespace fs = std::filesystem;
namespace bp = boost::process;

// COMMAND HANDLER FUNCTION DECLARATION ----------------------------------------

/**
 * @brief Command Switch / Handler Function
 * 
 * @param command String value of command to be ran.
 * @param args Vector array of arguments to send to the command
 * @return Int error code returned by the command. 
 * 
 * @note Native commands are {exit, echo, type}
 * 
 */
int handle_commands(
  std::string command, 
  std::vector<std::string> args);

// input HANDLER FUNCTION DECLARATION ----------------------------------------

/**
 * @brief Handle input to correct parseable format from a line.  
 * 
 * @param command_ptr Reference to string to place parsed command in.
 * @param args_ptr Reference to vector of strings to place parsed arguments in. 
 * @return 0
 * 
 * The function parses quotes and endline to use within the shell. 
 * 
 */
int handle_input(
  std::string line, 
  std::string* command_ptr, 
  std::vector<std::string>* args_ptr);

// EXPERIMENTAL

int tokenize(std::deque<std::string>* tokens_que);
int extract_next_command(
  std::deque<std::string>* tokens,
  std::string* command_ptr,
  std::vector<std::string>* args_ptr);

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
  while (true) {
    std::cout << prompt;

    std::deque<std::string> tokens;
    std::string command; 
    std::vector<std::string> args;     
    args.reserve(16);                  

    tokenize(&tokens);
    // std::cout << "[DEBUG] tokens: [";
    // for (std::string arg : tokens) {
    //   std::cout << arg << ",";
    // }
    // std::cout << "]" << std::endl;

    extract_next_command(&tokens, &command, &args);
    //std::cout << command << std::endl;
    

    // handle_input(line, &command, &args);

    //if empty skip else handle command
    try {
      err_code = handle_commands(command, args);
      if (err_code == -5) {break;} // CONVERT TO CATCH EXECPTION (ExitShell exception and clean exist)
    } 
    catch (const std::exception& e) {
      if (DEBUG != true) {
        std::cout << "Exception: \n" << e.what();
        std::cout << std::endl;
      } 
      else {
        throw;
      }
    }
  }
}


// COMMAND HANDLER FUNCTION ----------------------------------------------------

int tokenize( 
  std::deque<std::string>* tokens)
  {

  std::string n_token = "";
  bool is_token = false;
  bool dquoted  = false;
  bool squoted  = false;
  bool escaped  = false;
  bool requestline = false;
  bool addline;
  
  do {
    if (requestline) {
      requestline = !requestline;
      std::cout << "> ";
    }

    std::string line;
    std::getline(std::cin, line);
    
    // Edge cases [quoted and line is empty], [line ends with \]
    if (addline && line.empty()) {
      addline = !addline;
      break;
    }

    if ((squoted || dquoted) && line.empty()) {
      //std::cout << "[DEBUG] \\n added to token" << std::endl;
      n_token += '\n';
      std::cout << "> ";
      continue;
    }

    addline = (line.back() == '\\');
    if (addline) { 
      line.pop_back(); 
    }

    boost::trim(line);

    //std::cout << "[DEBUG] got line: [" << line << "]" << std::endl;

    // Parse the line
    for (std::string::iterator ch = line.begin(); ch !=line.end(); ++ch) {
      // Escaping chars
      //std::cout << "[DEBUG] n_token = [" << n_token << "]" << std::endl;

      if (escaped) {
        n_token += *ch;
        escaped = !escaped;
      }
      //------------------------------

      // Quoted Handling--------------------

      else if (*ch == '\'' && !dquoted && squoted && std::next(ch) == line.end()) {
        //std::cout << "[DEBUG] un_squoted at end of line, r_line" << requestline << std::endl; 
        squoted = !squoted;
        is_token = true;
      }
      else if (*ch == '\'' && !dquoted && std::next(ch) == line.end()) {
        n_token += '\n';
        squoted = !squoted;
        requestline = true;
      } 
      // find ' outside "" 
      else if (*ch == '\'' && !dquoted) {
        squoted = !squoted;
      } 
      // handle '' across lines. 
      else if (squoted) {
        n_token += *ch;
        if (std::next(ch) == line.end()) {
          //std::cout << "[DEBUG] squoted at end of line" << std::endl;
          n_token += '\n';
          requestline = true;
        }
      }

      // only process escapes outside of ''
      else if (*ch == '\\') {
        escaped = !escaped;
      }

      // find ""
      else if (*ch == '\"' && dquoted && std::next(ch) == line.end()) {
        //std::cout << "[DEBUG] un_dquoted at end of line, r_line" << requestline << std::endl; 
        dquoted = !dquoted;
        is_token = true;
      }
      else if (*ch == '\"' && std::next(ch) == line.end()) {
        n_token += '\n';
        dquoted = !dquoted;
        requestline = true;
      } 
      else if (*ch == '\"') {
        dquoted = !dquoted;
      }

      // handle "" across lines

      else if (dquoted) {
        n_token += *ch;
        if (std::next(ch) == line.end()) {
          //std::cout << "[DEBUG] dquoted at end of line" << std::endl;
          n_token += '\n';
          requestline = true;
        }
      }
      //-----------------------------------

      // Normal chars
      else if (std::next(ch) == line.end()) {
        n_token += *ch;
        is_token = true;
      }  
      else if (!isspace(*ch)) {
        n_token += *ch;
        is_token = false;
      }
      else {
        is_token = true;
      }
      
      if (requestline) {break;}
      if (!is_token || n_token.length() == 0) {continue;}
      
      // ADD ARGUMENTS ---------------------#
      tokens->push_back(n_token);
      n_token.clear();
      is_token = false;
    }
    if (addline) {
      std::cout << "> ";
    }

  } while (squoted || dquoted || addline);
  
  return 0;
}

int handle_commands(std::string command, std::vector<std::string> args) {
  // list of default commands that handle_commands handles natively. 

  if (command.empty()) {
    return 0;
  }

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

int extract_next_command(
  std::deque<std::string>* tokens,
  std::string* command_ptr,
  std::vector<std::string>* args_ptr) {

  if (tokens->empty()) {
    return 0;
  }
  *command_ptr = tokens->front();
  tokens->pop_front();
  while (!tokens->empty()) {
     args_ptr->push_back(tokens->front());
     tokens->pop_front();
  }
  
  return 0;
}