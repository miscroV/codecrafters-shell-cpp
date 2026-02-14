/*******************************************************************************
 * SHELL BUILTIN FUNCTIONS IMPLEMENTATION
*******************************************************************************/

#include "shell-functions.h"

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <deque>
#include <boost/algorithm/string.hpp>

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

// SHELL Functions -------------------------------------------------------------

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

    // Parse line tokens --------------------------------
    for (std::string::iterator ch = line.begin(); ch !=line.end(); ++ch) {
      // Escaping char handling
      if (escaped) {
        n_token += *ch;
        escaped = !escaped;
      }
      //------------------------------

      // Quoted Handling #######################################################
      // Single Quotes handling ---------------
      else if (*ch == '\'' && !dquoted && squoted && std::next(ch) == line.end()) {
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
      else if (*ch == '\"' && dquoted && std::next(ch) == line.end()) {
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