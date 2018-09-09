#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "log.h"
#include "helper/qt.h"

using std::cerr;
using std::cout;
using std::dec;
using std::endl;
using std::ofstream;
using std::ostream;
using std::ostringstream;
using std::setw;
using std::strerror;
using std::string;
using std::to_string;
using std::chrono::steady_clock;

class NullLogger : public Logger
{
public:
  NullLogger() = default;

  Logger *prefix(const char * /*file*/, int /*line*/) override { return this; }

  ostream &stream() override { return unopened; }

private:
  ofstream unopened;
};

class FileLogger : public Logger
{
public:
  FileLogger(const char *filename) : log_stream{filename, std::ios::out | std::ios::app}
  {
    if (!log_stream) {
      int stream_errno = errno;

      ostringstream msg;
      msg << "Unable to log to " << filename << ": " << strerror(stream_errno);
      err = msg.str();
    }

    FileLogger::prefix(__FILE__, __LINE__);
    log_stream << "FileLogger opened." << endl;
  }

  Logger *prefix(const char *file, int line) override
  {
    log_stream << "[" << setw(15) << file << ":" << setw(3) << dec << line << "] ";
    return this;
  }

  ostream &stream() override { return log_stream; }

  string get_error() const override { return err; }

private:
  ofstream log_stream;
  string err;
};

class StderrLogger : public Logger
{
public:
  StderrLogger()
  {
    StderrLogger::prefix(__FILE__, __LINE__);
    cerr << "StderrLogger opened." << endl;
  }

  Logger *prefix(const char *file, int line) override
  {
    cerr << "[" << setw(15) << file << ":" << setw(3) << dec << line << "] ";
    return this;
  }

  ostream &stream() override { return cerr; }

  string get_error() const override
  {
    if (!cerr) {
      return "Unable to log to stderr";
    }

    return "";
  }
};

class StdoutLogger : public Logger
{
public:
  StdoutLogger()
  {
    StdoutLogger::prefix(__FILE__, __LINE__);
    cout << "StdoutLogger opened." << endl;
  }

  Logger *prefix(const char *file, int line) override
  {
    cout << "[" << setw(15) << file << ":" << setw(3) << dec << line << "] ";
    return this;
  }

  ostream &stream() override { return cout; }

  string get_error() const override
  {
    if (!cout) {
      return "Unable to log to stdout";
    }

    return "";
  }
};

QThreadStorage<Logger *> loggers;

Logger *Logger::current()
{

  auto *logger = loggers.localData();

  if (logger == nullptr) {
    logger = new NullLogger();
    loggers.setLocalData(logger);
  }

  return logger;
}

string replace_logger(const Logger *new_logger)
{
    if(new_logger)
    {
        string r = new_logger->get_error();
        if (!r.empty()) {
          delete new_logger;
        }
        return r;
    }

  loggers.setLocalData((Logger *)new_logger);
  return "";
}

string Logger::to_file(const char *filename)
{
  return replace_logger(new FileLogger(filename));  // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
}

string Logger::to_stderr()
{
  return replace_logger(new StderrLogger());  // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
}

string Logger::to_stdout()
{
  return replace_logger(new StdoutLogger());  // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
}

string Logger::disable()
{
  return replace_logger(nullptr);
}

string Logger::from_env(const char *varname)
{
  const char *value = std::getenv(varname);
  if (value == nullptr) {
    return replace_logger(nullptr);
  }

  if (std::strcmp("stdout", value) == 0) {
    return to_stdout();
  }
  if (std::strcmp("stderr", value) == 0) {
    return to_stderr();
  }
  return to_file(value);
}

string plural(long quantity, const string &singular_form, const string &plural_form)
{
  string result;
  result += to_string(quantity);
  result += " ";

  if (quantity == 1) {
    result += singular_form;
  } else {
    result += plural_form;
  }

  return result;
}

string plural(long quantity, const string &singular_form)
{
  string plural_form(singular_form + "s");
  return plural(quantity, singular_form, plural_form);
}

FsTimer::FsTimer() : start{steady_clock::now()}, duration{0}
{
  //
}

void FsTimer::stop()
{
  duration = measure_duration();
}

string FsTimer::format_duration() const
{
  size_t total = duration;
  if (total == 0) {
    total = measure_duration();
  }

  size_t milliseconds = total;
  size_t seconds = milliseconds / 1000;
  milliseconds -= (seconds * 1000);

  size_t minutes = seconds / 60;
  seconds -= (minutes * 60);

  size_t hours = minutes / 60;
  minutes -= (hours * 60);

  ostringstream out;
  if (hours > 0) out << plural(hours, "hour") << ' ';
  if (minutes > 0) out << plural(minutes, "minute") << ' ';
  if (seconds > 0) out << plural(seconds, "second") << ' ';
  out << plural(milliseconds, "millisecond") << " (" << total << "ms)";
  return out.str();
}
