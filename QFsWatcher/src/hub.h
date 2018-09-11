#ifndef HUB_H
#define HUB_H

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "errable.h"
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

  Result<> worker_cache_size(size_t cache_size, PendingCallback &&callback)
  {
    if (!check_async(callback)) return ok_result();

    return send_command(worker_thread, CommandPayloadBuilder::cache_size(cache_size), std::move(callback));
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
