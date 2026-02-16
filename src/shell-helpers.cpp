/*******************************************************************************
 * SHELL HELPERS
*******************************************************************************/

#include <boost/process.hpp>
#include <filesystem>
#include <string>
#include <boost/asio.hpp>
#include <fstream>
#include <iostream>

// HELPER FUNCTIONS ------------------------------------------------------------

void read_async(boost::process::async_pipe& pipe, std::ostream& out_pipe) {
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
  boost::process::async_pipe o_pipe(io);
  boost::process::async_pipe e_pipe(io);

  args.insert(args.begin(), command); 

  if (r_stderr && r_stdout) {
    boost::process::child c(
      boost::process::args(args),
      boost::process::std_out > o_pipe,
      boost::process::std_err > e_pipe,
      boost::process::std_in  < stdin
    );
    read_async(o_pipe, std::cout);
    read_async(e_pipe, std::cerr);
    io.run();
    return c.exit_code();
  }

  if (r_stdout) {
    boost::process::child c(
      boost::process::args(args),
      boost::process::std_out > o_pipe,
      boost::process::std_err > stderr,
      boost::process::std_in  < stdin
    );
    read_async(o_pipe, std::cout);
    io.run();
    return c.exit_code();
  }

  if (r_stderr) {
    boost::process::child c(
      boost::process::args(args),
      boost::process::std_out > stdout,
      boost::process::std_err > e_pipe,
      boost::process::std_in  < stdin
    );
    read_async(e_pipe, std::cerr);
    io.run();
    return c.exit_code();
  }

  boost::process::child c(
    boost::process::args(args),
    boost::process::std_out > stdout,
    boost::process::std_err > stdin,
    boost::process::std_in  < stdin
  );
  c.wait();
  return c.exit_code();
}

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