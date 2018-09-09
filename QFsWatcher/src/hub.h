#ifndef HUB_H
#define HUB_H

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "errable.h"
#include "log.h"
#include "message.h"
#include "polling/polling_thread.h"
#include "result.h"
#include "worker/worker_thread.h"
#include "helper/qt.h"

class Hub : public Errable
{
public:
    typedef std::function<void(std::unique_ptr<std::string>, std::unique_ptr<ChannelID>)> PendingCallback;
    typedef std::function<void(std::unique_ptr<std::string>, std::unique_ptr<std::vector<FileSystemEvent>>)> ChannelCallback;
public:
  static Hub &get();

  Hub(const Hub &) = delete;
  Hub(Hub &&) = delete;
  ~Hub() override = default;

  Hub &operator=(const Hub &) = delete;
  Hub &operator=(Hub &&) = delete;

  Result<> use_main_log_file(std::string &&main_log_file)
  {
    Result<> h = health_err_result();
    if (h.is_error()) return h;

    std::string r = Logger::to_file(main_log_file.c_str());
    return r.empty() ? ok_result() : error_result(std::move(r));
  }

  Result<> use_main_log_stderr()
  {
    Result<> h = health_err_result();
    if (h.is_error()) return h;

    std::string r = Logger::to_stderr();
    return r.empty() ? ok_result() : error_result(std::move(r));
  }

  Result<> use_main_log_stdout()
  {
    Result<> h = health_err_result();
    if (h.is_error()) return h;

    std::string r = Logger::to_stdout();
    return r.empty() ? ok_result() : error_result(std::move(r));
  }

  Result<> disable_main_log()
  {
    Result<> h = health_err_result();
    if (h.is_error()) return h;

    std::string r = Logger::disable();
    return r.empty() ? ok_result() : error_result(std::move(r));
  }

  Result<> use_worker_log_file(std::string &&worker_log_file, PendingCallback &&callback)
  {
    if (!check_async(callback)) return ok_result();

    return send_command(
      worker_thread, CommandPayloadBuilder::log_to_file(std::move(worker_log_file)), std::move(callback));
  }

  Result<> use_worker_log_stderr(PendingCallback &&callback)
  {
    if (!check_async(callback)) return ok_result();

    return send_command(worker_thread, CommandPayloadBuilder::log_to_stderr(), std::move(callback));
  }

  Result<> use_worker_log_stdout(PendingCallback &&callback)
  {
    if (!check_async(callback)) return ok_result();

    return send_command(worker_thread, CommandPayloadBuilder::log_to_stdout(), std::move(callback));
  }

  Result<> disable_worker_log(PendingCallback &&callback)
  {
    if (!check_async(callback)) return ok_result();

    return send_command(worker_thread, CommandPayloadBuilder::log_disable(), std::move(callback));
  }

  Result<> worker_cache_size(size_t cache_size, PendingCallback &&callback)
  {
    if (!check_async(callback)) return ok_result();

    return send_command(worker_thread, CommandPayloadBuilder::cache_size(cache_size), std::move(callback));
  }

  Result<> use_polling_log_file(std::string &&polling_log_file, PendingCallback &&callback)
  {
    if (!check_async(callback)) return ok_result();

    return send_command(
      polling_thread, CommandPayloadBuilder::log_to_file(std::move(polling_log_file)), std::move(callback));
  }

  Result<> use_polling_log_stderr(PendingCallback &&callback)
  {
    if (!check_async(callback)) return ok_result();

    return send_command(polling_thread, CommandPayloadBuilder::log_to_stderr(), std::move(callback));
  }

  Result<> use_polling_log_stdout(PendingCallback &&callback)
  {
    if (!check_async(callback)) return ok_result();

    return send_command(polling_thread, CommandPayloadBuilder::log_to_stdout(), std::move(callback));
  }

  Result<> disable_polling_log(PendingCallback &&callback)
  {
    if (!check_async(callback)) return ok_result();

    return send_command(polling_thread, CommandPayloadBuilder::log_disable(), std::move(callback));
  }

  Result<> set_polling_interval(uint_fast32_t interval, PendingCallback &&callback)
  {
    if (!check_async(callback)) return ok_result();

    return send_command(polling_thread, CommandPayloadBuilder::polling_interval(interval), std::move(callback));
  }

  Result<> set_polling_throttle(uint_fast32_t throttle, PendingCallback &&callback)
  {
    if (!check_async(callback)) return ok_result();

    return send_command(polling_thread, CommandPayloadBuilder::polling_throttle(throttle), std::move(callback));
  }

  Result<> watch(std::string &&root,
    bool poll,
    bool recursive,
    PendingCallback &&ack_callback,
    ChannelCallback &&event_callback);

  Result<> unwatch(ChannelID channel_id, PendingCallback &&ack_callback);

  void handle_events();

private:

  Hub();

  Result<> send_command(Thread &thread, CommandPayloadBuilder &&builder, PendingCallback &&callback);

  bool check_async(const PendingCallback &callback);

  void handle_events_from(Thread &thread);

  WorkerThread worker_thread;
  PollingThread polling_thread;
  QtMainThreadCallback main_callback;

  CommandID next_command_id;
  ChannelID next_channel_id;
  RequestID next_request_id;

  std::unordered_map<CommandID, PendingCallback> pending_callbacks;
  std::unordered_map<ChannelID, ChannelCallback> channel_callbacks;
};

#endif
