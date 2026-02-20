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

#include <bitset>
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

enum tokinizationFlags : std::uint8_t {
  None = 0,
  SQUOTED = 1 << 0,
  DQUOTED = 1 << 1,
  ESCAPED = 1 << 2,
  REQLINE = 1 << 3,
  ADDLINE = 1 << 4,
};

int gather_input(std::string* line) {
  char c;
  while (read(STDIN_FILENO, &c, 1) == 1) {
    if (c == '\n') {
      break;
    }
    if (c == 127 && !line->empty()) {
      line->pop_back();
      printf("\b \b");
      fflush(stdout);
    }
    if (c == 9) {
      std::deque<std::string> t_tokens;
      tokenize_input(&t_tokens, *line);
      std::cout << t_tokens.back();
    }
    else if (iscntrl(c)) {
      
    } 
    else {
      printf("%c", c);
      line->append(1, c);
      fflush(stdout);
    }
  }
  printf("\n");
  fflush(stdout);
  return 0;
}

int tokenize_input_loop(std::deque<std::string>* tokens) {
  int flags = tokinizationFlags::None;
  bool dquoted  = (flags & DQUOTED);
  bool squoted  = (flags & SQUOTED);
  bool escaped  = (flags & ESCAPED);
  bool requestline = (flags & REQLINE);
  bool addline  = (flags & ADDLINE);
  do {
    if (requestline) {
      requestline = !requestline;
      flags ^= REQLINE;
      std::cout << "> ";
    }
    std::string line;
    gather_input(&line);
    

    flags = tokenize_input(tokens, line, flags);
    
    dquoted  = (flags & DQUOTED);
    squoted  = (flags & SQUOTED);
    escaped  = (flags & ESCAPED);
    requestline = (flags & REQLINE);
    addline  = (flags & ADDLINE);

    if (addline) {
      std::cout << "> ";
    }

  } while (squoted || dquoted || addline);
  return 0;
}

int tokenize_input( 
  std::deque<std::string>* tokens,
  std::string line,
  int flags )
  {
  std::string n_token = "";
  bool is_token = false;
  bool dquoted  = (flags & DQUOTED);
  bool squoted  = (flags & SQUOTED);
  bool escaped  = (flags & ESCAPED);
  bool requestline = (flags & REQLINE);
  bool addline  = (flags & ADDLINE);
  

  // Gather a new line -----------------------------
  if (requestline) {
    requestline = !requestline;
    flags ^= REQLINE;
    std::cout << "> ";
  }

  // Handle special line forms ----------------------
  if (addline && line.empty()) {
    addline = !addline;
    flags ^= ADDLINE;
    return flags;
  }
  if ((squoted || dquoted) && line.empty()) {
    n_token += '\n';
    std::cout << "> ";
    return flags;
  }
  if (line.empty()) {
    return flags;
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
      flags ^= ESCAPED;
    }
    //------------------------------

    // Quoted Handling #######################################################
    // Single Quotes handling ---------------
    else if (*ch == '\'' && !dquoted && std::next(ch) == line.end() && squoted) {
      squoted = !squoted;
      flags ^= SQUOTED;
      is_token = true;
    }
    else if (*ch == '\'' && !dquoted && std::next(ch) == line.end()) {
      n_token += '\n';
      squoted = !squoted;
      flags ^= SQUOTED;
      requestline = true;
      flags |= REQLINE;
    } 
    // find ' outside "" 
    else if (*ch == '\'' && !dquoted) {
      squoted = !squoted;
      flags ^= SQUOTED;
    } 
    // handle '' across lines. 
    else if (squoted) {
      n_token += *ch;
      if (std::next(ch) == line.end()) {
        n_token += '\n';
        requestline = true;
        flags ^= REQLINE;
      }
    }
    // -------------------------------------

    // Escape \ outside '' handling --------
    else if (*ch == '\\') { escaped = !escaped; flags ^= ESCAPED;}

    // Double Quotes Handling ---------------
    else if (*ch == '\"'  && std::next(ch) == line.end() && dquoted) {
      dquoted = !dquoted;
      flags ^= DQUOTED;
      is_token = true;
    }
    else if (*ch == '\"' && std::next(ch) == line.end()) {
      n_token += '\n';
      dquoted = !dquoted;
      flags ^= DQUOTED;
      requestline = true;
      flags ^= REQLINE;
    } 
    else if (*ch == '\"') {
      dquoted = !dquoted;
      flags ^= DQUOTED;
    }
    else if (dquoted) {
      n_token += *ch;
      if (std::next(ch) == line.end()) {
        n_token += '\n';
        requestline = true;
        flags |= REQLINE;
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
    if (requestline) {
      tokens->push_back(n_token);
      break;
    }
    if (!is_token || n_token.length() == 0) {continue;}
    tokens->push_back(n_token);
    n_token.clear();
    is_token = false;
    // --------------------------------------
  }
  // if (addline) {
  //   std::cout << "> ";
  // }
  // std::cout << "[DEBUG] flags : " << std::bitset<8>(flags) << std::endl;
  
  return flags;
}

int tokenize_input( 
  std::deque<std::string>* tokens,
  std::string line) {
    return tokenize_input(tokens, line, tokinizationFlags::None);
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