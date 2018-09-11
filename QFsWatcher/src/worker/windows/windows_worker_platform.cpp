#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>

#include "../../helper/windows/helper.h"
#include "../../message.h"
#include "../../message_buffer.h"
#include "../recent_file_cache.h"
#include "../worker_platform.h"
#include "../worker_thread.h"
#include "subscription.h"
#include "../../stringbuilder.h"

using std::default_delete;
using std::endl;
using std::make_pair;
using std::map;
using std::move;
using std::pair;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;
using std::wstring;

const size_t DEFAULT_CACHE_SIZE = 4096;

const size_t DEFAULT_CACHE_PREPOPULATION = 1024;

void CALLBACK command_perform_helper(__in ULONG_PTR payload);

void CALLBACK event_helper(DWORD error_code, DWORD num_bytes, LPOVERLAPPED overlapped);

class WindowsWorkerPlatform : public WorkerPlatform
{
public:
  WindowsWorkerPlatform(WorkerThread *thread) : WorkerPlatform(thread), thread_handle{0}, cache{DEFAULT_CACHE_SIZE}
  {
    freeze();
  };

  ~WindowsWorkerPlatform() override {}

  Result<> wake() override
  {
    std::lock_guard<std::mutex> lock(thread_handle_mutex);

    if (!thread_handle) {
      return ok_result();
    }

    BOOL success = QueueUserAPC(command_perform_helper, thread_handle, reinterpret_cast<ULONG_PTR>(this));
    if (!success) {
      return windows_error_result<>("Unable to queue APC");
    }

    return ok_result();
  }

  Result<> init() override
  {
    std::lock_guard<std::mutex> lock(thread_handle_mutex);

    HANDLE pseudo_handle = GetCurrentThread();
    BOOL success = DuplicateHandle(GetCurrentProcess(),  // Source process
      pseudo_handle,  // Source handle
      GetCurrentProcess(),  // Destination process
      &thread_handle,  // Destination handle
      0,  // Desired access
      FALSE,  // Inheritable by new processes
      DUPLICATE_SAME_ACCESS  // options
    );
    if (!success) {
      return windows_error_result<>("Unable to duplicate thread handle");
    }

    return ok_result();
  }

  Result<> listen() override
  {
    while (true) {
      SleepEx(INFINITE, true);
    }

    return error_result("listen loop ended unexpectedly");
  }

  Result<bool> handle_add_command(CommandID command,
    ChannelID channel,
    const string &root_path,
    bool recursive) override
  {
    // Convert the path to a wide-character string
    Result<wstring> convr = to_wchar(root_path);
    if (convr.is_error()) return convr.propagate<bool>();
    wstring &root_path_w = convr.get_value();

    // Open a directory handle
    HANDLE root = CreateFileW(root_path_w.c_str(),  // null-terminated wchar file name
      FILE_LIST_DIRECTORY,  // desired access
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,  // share mode
      NULL,  // security attributes
      OPEN_EXISTING,  // creation disposition
      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,  // flags and attributes
      NULL  // template file
    );
    if (root == INVALID_HANDLE_VALUE) {
      stringbuilder msg;
      msg << "Unable to open directory handle " << root_path;
      return windows_error_result<bool>(msg.str());
    }

    // Allocate and persist the subscription
    Subscription *sub = new Subscription(channel, root, root_path_w, recursive, this);
    auto insert_result = subscriptions.insert(make_pair(channel, sub));
    if (!insert_result.second) {
      delete sub;

	  stringbuilder msg;
	  msg << "Channel collision: " << channel;
      return Result<bool>::make_error(msg.mstr());
    }

    Result<bool> schedr = sub->schedule(&event_helper);
    if (schedr.is_error()) return schedr.propagate<bool>();
    if (!schedr.get_value()) {
      return emit_msg(Message(CommandPayloadBuilder::add(channel, string(root_path), recursive, 1).build()))
        .propagate(false);
    }

    cache.prepopulate(root_path, DEFAULT_CACHE_PREPOPULATION, recursive);

    return ok_result(true);
  }

  Result<bool> handle_remove_command(CommandID command, ChannelID channel) override
  {
    auto it = subscriptions.find(channel);
    if (it == subscriptions.end()) {
      return ok_result(true);
    }

    Result<> r = it->second->stop(command);
    if (r.is_error()) return r.propagate<bool>();

    return ok_result(false);
  }

  void handle_cache_size_command(size_t cache_size) override
  {
    cache.resize(cache_size);
  }

  Result<> handle_fs_event(DWORD error_code, DWORD num_bytes, Subscription *sub)
  {
    // Ensure that the subscription is valid.
    ChannelID channel = sub->get_channel();
    auto it = subscriptions.find(channel);
    if (it == subscriptions.end() || it->second != sub) {
      return ok_result();
    }

    // Subscription termination.
    bool terminate = false;
    if (error_code == ERROR_OPERATION_ABORTED) {
      terminate = true;
    } else if (sub->is_terminating()) {
      terminate = true;
    }
    if (terminate) return remove(sub);

    // Handle errors.
    if (error_code == ERROR_INVALID_PARAMETER) {

      Result<> resize = sub->use_network_size();
      if (resize.is_error()) return emit_fatal_error(sub, move(resize));

      return reschedule(sub);
    }

    if (error_code == ERROR_NOTIFY_ENUM_DIR) {
      return reschedule(sub);
    }

    if (error_code != ERROR_SUCCESS) {
      return emit_fatal_error(sub, windows_error_result<>("Completion callback error", error_code));
    }

    if (num_bytes == 0) {
      return reschedule(sub);
    }

    // Schedule the next completion callback.
    BYTE *base = sub->get_written(num_bytes);
    Result<> next = reschedule(sub);

    // Process received events.
    MessageBuffer buffer;
    ChannelMessageBuffer messages(buffer, channel);
    size_t num_events = 0;

    while (true) {
      PFILE_NOTIFY_INFORMATION info = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(base);
      num_events++;

      Result<> pr = process_event_payload(info, sub, messages);
      if (pr.is_error()) {
      }

      if (info->NextEntryOffset == 0) {
        break;
      }
      base += info->NextEntryOffset;
    }

    cache.apply();
    cache.prune();

    if (!messages.empty()) {
      Result<> er = emit_all(messages.begin(), messages.end());
    }

    return next.propagate_as_void();
  }

private:
  Result<> reschedule(Subscription *sub)
  {
    Result<bool> sch = sub->schedule(&event_helper);
    if (sch.is_error()) return emit_fatal_error(sub, sch.propagate_as_void());

    if (!sch.get_value()) {
      Result<string> root = sub->get_root_path();
      if (root.is_error()) return emit_fatal_error(sub, root.propagate_as_void());

      Result<> rem = remove(sub);
      rem &= emit_msg(Message(
        CommandPayloadBuilder::add(sub->get_channel(), move(root.get_value()), sub->is_recursive(), 1).build()));
      return rem;
    }

    return ok_result();
  }

  Result<> remove(Subscription *sub)
  {
    Message response(AckPayload(sub->get_command_id(), sub->get_channel(), true, ""));

    // Ensure that the subscription is valid.
    ChannelID channel = sub->get_channel();
    auto it = subscriptions.find(channel);
    if (it == subscriptions.end() || it->second != sub) {
      return ok_result();
    }

    subscriptions.erase(it);
    delete sub;

    if (sub->get_command_id() != NULL_COMMAND_ID) {
      return emit_msg(move(response));
    } else {
      return ok_result();
    }
  }

  Result<> emit_fatal_error(Subscription *sub, Result<> &&r)
  {
    assert(r.is_error());

    Result<> out = emit_msg(Message(ErrorPayload(sub->get_channel(), string(r.get_error()), true)));
    out &= remove(sub);
    return out;
  }

  Result<> process_event_payload(PFILE_NOTIFY_INFORMATION info, Subscription *sub, ChannelMessageBuffer &messages)
  {
    ChannelID channel = sub->get_channel();
    wstring relpathw{info->FileName, info->FileNameLength / sizeof(WCHAR)};
    wstring shortpathw = sub->make_absolute(move(relpathw));
    Result<wstring> longpathr = to_long_path(shortpathw);
    if (longpathr.is_error()) return longpathr.propagate_as_void();
    wstring &pathw = longpathr.get_value();

    Result<string> u8r = to_utf8(pathw);
    if (u8r.is_error()) {
      return ok_result();
    }
    string &path = u8r.get_value();

    shared_ptr<StatResult> stat;
    if (info->Action == FILE_ACTION_REMOVED || info->Action == FILE_ACTION_RENAMED_OLD_NAME) {
      stat = cache.former_at_path(path, false, false, false);
    } else {
      stat = cache.current_at_path(path, false, false, false);
      if (stat->is_absent()) {
        stat = cache.former_at_path(path, false, false, false);
      }
    }
    EntryKind kind = stat->get_entry_kind();

    switch (info->Action) {
      case FILE_ACTION_ADDED:
        messages.created(move(path), kind);
        break;
      case FILE_ACTION_MODIFIED:
        if (kind != KIND_DIRECTORY) {
          messages.modified(move(path), kind);
        } else {
        }
        break;
      case FILE_ACTION_REMOVED:
        messages.deleted(move(path), kind);
        break;
      case FILE_ACTION_RENAMED_OLD_NAME:
        sub->remember_old_path(move(path), kind);
        break;
      case FILE_ACTION_RENAMED_NEW_NAME:
        if (sub->was_old_path_seen()) {
          // Old name received first
          if (kind == KIND_UNKNOWN) {
            kind = sub->get_old_path_kind();
          }

          cache.update_for_rename(sub->get_old_path(), path);
          messages.renamed(string(sub->get_old_path()), move(path), kind);
          sub->clear_old_path();
        } else {
          // No old name. Treat it as a creation
          messages.created(move(path), kind);
        }
        break;
      default:
        break;
    }

    return ok_result();
  }

  std::mutex thread_handle_mutex;
  HANDLE thread_handle;

  map<ChannelID, Subscription *> subscriptions;

  RecentFileCache cache;
};

unique_ptr<WorkerPlatform> WorkerPlatform::for_worker(WorkerThread *thread)
{
  return unique_ptr<WorkerPlatform>(new WindowsWorkerPlatform(thread));
}

void CALLBACK command_perform_helper(__in ULONG_PTR payload)
{
  WindowsWorkerPlatform *platform = reinterpret_cast<WindowsWorkerPlatform *>(payload);
  platform->handle_commands();
}

void CALLBACK event_helper(DWORD error_code, DWORD num_bytes, LPOVERLAPPED overlapped)
{
  Subscription *sub = static_cast<Subscription *>(overlapped->hEvent);
  Result<> r = sub->get_platform()->handle_fs_event(error_code, num_bytes, sub);
}
