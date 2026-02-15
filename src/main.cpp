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
#include <boost/asio.hpp>
#include <iomanip>
#include <deque>
#include <fstream>

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
  std::vector<std::string> args,
  bool r_stdout,
  bool r_stderr);

// EXPERIMENTAL
int run_path_command(
  std::string command, 
  std::vector<std::string> args,
  bool r_stdout,
  bool r_stderr);

int extract_next_command(
  std::deque<std::string>* tokens,
  std::string* command_ptr,
  std::vector<std::string>* args_ptr,
  std::ofstream* r_stdout,
  std::ofstream* r_stderr);

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
    
    std::ofstream r_stdout;
    std::ofstream r_stderr;
    std::streambuf* stdout_buf;


    tokenize_input(&tokens);
    extract_next_command(&tokens, &command, &args, &r_stdout, &r_stderr);

    // std::cout << "[DEBUG] tokens: [";
    // for (std::string arg : args) {
    //   std::cout << arg << ",";
    // }
    // std::cout << "]" << std::endl;
    
    if (r_stdout.is_open()) {
      stdout_buf = std::cout.rdbuf();
      std::cout.rdbuf(r_stdout.rdbuf());
    }

    //if empty skip else handle command
    try {
      err_code = handle_commands(command, args, r_stdout.is_open(), r_stderr.is_open());
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

    if (r_stdout.is_open()) {
      std::cout.rdbuf(stdout_buf);
      r_stdout.close();
    }
  }
}


// COMMAND HANDLER FUNCTION ----------------------------------------------------

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
  else if (fs::path execpath = get_executable_path(command); 
    !execpath.empty()) { 
      return run_path_command(command, args, r_stdout, r_stdin); 
    }
  // if command not found:
  else {
    std::cout << command << ": command not found" << std::endl;
  return 0;
  }
}   

void read_async(bp::async_pipe& pipe, std::ostream& out_pipe) {
  static boost::asio::streambuf buf;
  static std::string line;
  boost::asio::async_read_until(
    pipe, buf, '\n',
    [&](boost::system::error_code ec, size_t n) {
      if (ec == boost::asio::error::eof) {
        return;
      } 
      else if (ec) {
        out_pipe << ec.message() << std::endl;
        return;
      }
      if (getline(std::istream(&buf), line)) {
        out_pipe << line << std::endl;
      }
      read_async(pipe, out_pipe);
    }
  );
} 

int run_path_command(
  std::string command, 
  std::vector<std::string> args, 
  bool r_stdout,
  bool r_stderr) {

  boost::asio::io_context io;
  bp::async_pipe o_pipe(io);
  bp::async_pipe e_pipe(io);

  args.insert(args.begin(), command); 

  if (r_stderr && r_stdout) {
    bp::child c(
      bp::args(args),
      bp::std_out > o_pipe,
      bp::std_err > e_pipe,
      bp::std_in  < stdin
    );
    read_async(o_pipe, std::cout);
    read_async(e_pipe, std::cerr);
    io.run();
    return c.exit_code();
  }

  if (r_stdout) {
    bp::child c(
      bp::args(args),
      bp::std_out > o_pipe,
      bp::std_err > stderr,
      bp::std_in  < stdin
    );
    read_async(o_pipe, std::cout);
    io.run();
    return c.exit_code();
  }

  if (r_stderr) {
    bp::child c(
      bp::args(args),
      bp::std_out > stdout,
      bp::std_err > e_pipe,
      bp::std_in  < stdin
    );
    read_async(e_pipe, std::cerr);
    io.run();
    return c.exit_code();
  }

  bp::child c(
    bp::args(args),
    bp::std_out > stdout,
    bp::std_err > stdin,
    bp::std_in  < stdin
  );
  c.wait();
  return c.exit_code();
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
  while (!tokens->empty()) {
    if (tokens->front() == ">" || tokens->front() == "1>") {
      redirect = true;
    }
    else if (redirect) {
      if (r_stdout->is_open()) {r_stdout->close();}
      r_stdout->open(tokens->front());
      redirect = false;
    }
    else {
      args_ptr->push_back(tokens->front());
    }
    tokens->pop_front();
  }
  return 0;
}