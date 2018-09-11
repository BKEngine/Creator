#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "hub.h"
#include "message.h"
#include "polling/polling_thread.h"
#include "result.h"
#include "status.h"
#include "worker/worker_thread.h"

using std::endl;
using std::map;
using std::move;
using std::multimap;
using std::set;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

Hub::Hub() :
  main_callback([this](){handle_events();}),
  worker_thread(&main_callback),
  polling_thread(&main_callback),
  next_command_id{NULL_COMMAND_ID + 1},
  next_channel_id{NULL_CHANNEL_ID + 1},
  next_request_id{NULL_REQUEST_ID + 1}
{
  report_errable(worker_thread);
  report_errable(polling_thread);

  report_if_error(worker_thread.run());
  freeze();
}

Hub &Hub::get() {
    static Hub the_hub;
    return the_hub;
}

Result<> Hub::watch(string &&root,
                    bool poll,
                    bool recursive,
                    PendingCallback &&ack_callback,
  ChannelCallback &&event_callback)
{
  if (!check_async(ack_callback)) return ok_result();

  ChannelID channel_id = next_channel_id;
  next_channel_id++;

  channel_callbacks.emplace(channel_id, move(event_callback));

  if (poll) {
    return send_command(
      polling_thread, CommandPayloadBuilder::add(channel_id, move(root), recursive, 1), move(ack_callback));
  }

  return send_command(
    worker_thread, CommandPayloadBuilder::add(channel_id, move(root), recursive, 1), move(ack_callback));
}

Result<> Hub::unwatch(ChannelID channel_id, PendingCallback &&ack_callback)
{
  if (!check_async(ack_callback)) return ok_result();

  string root;
  PendingCallback all = move(ack_callback);

  Result<> r = ok_result();
  r &= send_command(
    worker_thread, CommandPayloadBuilder::remove(channel_id), PendingCallback(all));
  r &= send_command(polling_thread,
    CommandPayloadBuilder::remove(channel_id),
    PendingCallback(all));

  auto maybe_event_callback = channel_callbacks.find(channel_id);
  if (maybe_event_callback == channel_callbacks.end()) {
    return r;
  }
  channel_callbacks.erase(maybe_event_callback);
  return r;
}

void Hub::handle_events()
{
  handle_events_from(worker_thread);
  handle_events_from(polling_thread);
}

Result<> Hub::send_command(Thread &thread, CommandPayloadBuilder &&builder, PendingCallback &&callback)
{
  CommandID command_id = next_command_id;
  builder.set_id(command_id);
  Message command(builder.build());
  pending_callbacks.emplace(command_id, move(callback));
  next_command_id++;

  Result<bool> sr = thread.send(move(command));
  if (sr.is_error()) return sr.propagate();
  if (sr.get_value()) handle_events();
  return ok_result();
}

bool Hub::check_async(const PendingCallback &callback)
{
  if (is_healthy()) return true;

  if(callback)
    callback(std::make_unique<std::string>(get_message()), nullptr);
  return false;
}

void Hub::handle_events_from(Thread &thread)
{
  bool repeat = true;

  unique_ptr<vector<Message>> accepted = thread.receive_all();
  if (!accepted) {
    // No events to process.
    return;
  }

  map<ChannelID, vector<FileSystemEvent>> to_deliver;
  multimap<ChannelID, std::string> errors;
  set<ChannelID> to_unwatch;

  for (Message &message : *accepted) {
    const AckPayload *ack = message.as_ack();
    if (ack != nullptr) {
      auto maybe_callback = pending_callbacks.find(ack->get_key());
      if (maybe_callback == pending_callbacks.end()) {
        continue;
      }

      PendingCallback callback = move(maybe_callback->second);
      pending_callbacks.erase(maybe_callback);

      ChannelID channel_id = ack->get_channel_id();
      if (ack->was_successful()) {
        if(callback)
          callback(nullptr, std::make_unique<ChannelID>(channel_id));
      } else {
        if(callback)
          callback(std::make_unique<std::string>(ack->get_message()), nullptr);
      }

      continue;
    }

    const FileSystemPayload *fs = message.as_filesystem();
    if (fs != nullptr) {
      ChannelID channel_id = fs->get_channel_id();

      to_deliver[channel_id].emplace_back(fs->get_event());
      continue;
    }

    const CommandPayload *command = message.as_command();
    if (command != nullptr) {
      if (command->get_action() == COMMAND_DRAIN) {
        Result<bool> dr = thread.drain();
        if (dr.is_error()) {
        } else if (dr.get_value()) {
          repeat = true;
        }
      } else if (command->get_action() == COMMAND_ADD && &thread == &worker_thread) {
        polling_thread.send(move(message));
      } else {
      }

      continue;
    }

    const ErrorPayload *error = message.as_error();
    if (error != nullptr) {
      const ChannelID &channel_id = error->get_channel_id();

      errors.emplace(channel_id, error->get_message());

      if (error->was_fatal()) {
        to_unwatch.insert(channel_id);
      }

      continue;
    }
  }

  for (auto &&pair : to_deliver) {
    const ChannelID &channel_id = pair.first;
    vector<FileSystemEvent> &events = pair.second;

    auto maybe_callback = channel_callbacks.find(channel_id);
    if (maybe_callback == channel_callbacks.end()) {
      continue;
    }
    ChannelCallback &callback = maybe_callback->second;

    if(callback)
      callback(nullptr, std::make_unique<vector<FileSystemEvent>>(move(events)));
  }

  for (auto &pair : errors) {
    const ChannelID &channel_id = pair.first;
    string &err = pair.second;

    auto maybe_callback = channel_callbacks.find(channel_id);

    if (maybe_callback == channel_callbacks.end()) {
      continue;
    }
    ChannelCallback &callback = maybe_callback->second;

    if(callback)
      callback(std::make_unique<string>(err), nullptr);
  }

  for (const ChannelID &channel_id : to_unwatch) {
    Result<> er = unwatch(channel_id, nullptr);
  }

  if (repeat) handle_events_from(thread);
}
