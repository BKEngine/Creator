#ifndef LOG_H
#define LOG_H

#include <chrono>
#include <memory>
#include <ostream>
#include <string>

class Logger
{
public:
  static Logger *current();

  static std::string to_file(const char *filename);

  static std::string to_stderr();

  static std::string to_stdout();

  static std::string disable();

  static std::string from_env(const char *varname);

  Logger() = default;

  virtual ~Logger() = default;

  virtual Logger *prefix(const char *file, int line) = 0;

  virtual std::ostream &stream() = 0;

  virtual std::string get_error() const { return ""; }

  Logger(const Logger &) = delete;
  Logger(Logger &&) = delete;
  Logger &operator=(const Logger &) = delete;
  Logger &operator=(Logger &&) = delete;
};

std::string plural(long quantity, const std::string &singular_form, const std::string &plural_form);

std::string plural(long quantity, const std::string &singular_form);

#define LOGGER (Logger::current()->prefix(__FILE__, __LINE__)->stream())

class FsTimer
{
public:
  FsTimer();

  ~FsTimer() = default;

  void stop();

  std::string format_duration() const;

  FsTimer(const FsTimer &) = delete;
  FsTimer(FsTimer &&) = delete;
  FsTimer &operator=(const FsTimer &) = delete;
  FsTimer &operator=(FsTimer &&) = delete;

private:
  size_t measure_duration() const
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
  }

  std::chrono::time_point<std::chrono::steady_clock> start;

  size_t duration;
};

inline std::ostream &operator<<(std::ostream &out, const FsTimer &t)
{
  return out << t.format_duration();
}

#endif
