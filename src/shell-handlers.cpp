/*******************************************************************************
 * SHELL HANDLERS
*******************************************************************************/

#include "shell-handlers.h"
#include "shell-helpers.h"
#include "shell-builtins.h"

#include <deque>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <boost/algorithm/string/trim.hpp>

// SHELL Functions -------------------------------------------------------------


int handle_commands(
  std::string command, 
  std::vector<std::string> args,
  bool r_stdout,
  bool r_stdin) {
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
  else if (std::filesystem::path execpath = get_executable_path(command); 
    !execpath.empty()) { 
      return run_path_command(command, args, r_stdout, r_stdin); 
    }
  // if command not found:
  else {
    std::cout << command << ": command not found" << std::endl;
  return 0;
  }
}   

int tokenize_input( 
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
    // Gather a new line -----------------------------
    if (requestline) {
      requestline = !requestline;
      std::cout << "> ";
    }
    std::string line;
    std::getline(std::cin, line, '\t');
    std::cout << "found a tab!" << std::endl;
    std::getline(std::cin, line);
    // Handle special line forms ----------------------
    if (addline && line.empty()) {
      addline = !addline;
      break;
    }
    if ((squoted || dquoted) && line.empty()) {
      n_token += '\n';
      std::cout << "> ";
      continue;
    }
    addline = (line.back() == '\\');
    if (addline) { 
      line.pop_back(); 
    }
    boost::trim(line);

    // Parse line tokens--------------------------------
    for (std::string::iterator ch = line.begin(); ch !=line.end(); ++ch) {
      // Escaping char handling
      if (escaped) {
        n_token += *ch;
        escaped = !escaped;
      }
      //------------------------------

      // Quoted Handling #######################################################
      // Single Quotes handling ---------------
      else if (*ch == '\'' && !dquoted && std::next(ch) == line.end() && squoted) {
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
          n_token += '\n';
          requestline = true;
        }
      }
      // -------------------------------------

      // Escape \ outside '' handling --------
      else if (*ch == '\\') { escaped = !escaped; }

      // Double Quotes Handling ---------------
      else if (*ch == '\"'  && std::next(ch) == line.end() && dquoted) {
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
      else if (dquoted) {
        n_token += *ch;
        if (std::next(ch) == line.end()) {
          n_token += '\n';
          requestline = true;
        }
      }
      // -------------------------------------

      // Normal chars ------------------------
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
      // -------------------------------------
      //########################################################################

      // token and new line handling ---------
      if (requestline) {break;}
      if (!is_token || n_token.length() == 0) {continue;}
      tokens->push_back(n_token);
      n_token.clear();
      is_token = false;
      // --------------------------------------
    }
    if (addline) {
      std::cout << "> ";
    }

  } while (squoted || dquoted || addline);
  
  return 0;
}

int extract_next_command(
  std::deque<std::string>* tokens,
  std::string* command_ptr,
  std::vector<std::string>* args_ptr,
  std::ofstream* r_stdout,
  std::ofstream* r_stderr) {

  if (tokens->empty()) {
    return 0;
  }

  *command_ptr = tokens->front();
  tokens->pop_front();

  bool redirect = false;
  bool redirect_err = false;
  bool redirect_append = false;
  bool redirect_err_append = false;
  while (!tokens->empty()) {
    if (tokens->front() == ">" || tokens->front() == "1>") {
      redirect = true;
    }
    else if (redirect) {
      if (r_stdout->is_open()) {r_stdout->close();}
      r_stdout->open(tokens->front());
      redirect = false;
    }

    else if (tokens->front() == "2>") {
      redirect_err = true;
    }
    else if (redirect_err) {
      if (r_stderr->is_open()) {r_stderr->close();}
      r_stderr->open(tokens->front());
      redirect_err = false;
    }

    else if (tokens->front() == ">>" || tokens->front() == "1>>") {
      redirect_append = true;
    }
    else if (redirect_append) {
      if (r_stdout->is_open()) {r_stdout->close();}
      r_stdout->open(tokens->front(), std::ios::app);
      redirect_append = false;
    }

    else if (tokens->front() == "2>>") {
      redirect_err_append = true;
    }
    else if (redirect_err_append) {
      if (r_stderr->is_open()) {r_stderr->close();}
      r_stderr->open(tokens->front(), std::ios::app);
      redirect_err_append = false;
    }
    
    else {
      args_ptr->push_back(tokens->front());
    }
    tokens->pop_front();
  }
  return 0;
}