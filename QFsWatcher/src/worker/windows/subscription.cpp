#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <windows.h>

#include "../../helper/windows/helper.h"
#include "../../result.h"
#include "../../stringbuilder.h"
#include "subscription.h"

using std::endl;
using std::string;
using std::wstring;

const DWORD DEFAULT_BUFFER_SIZE = 128 * 1024;
const DWORD NETWORK_BUFFER_SIZE = 64 * 1024;

Subscription::Subscription(ChannelID channel,
  HANDLE root,
  const wstring &path,
  bool recursive,
  WindowsWorkerPlatform *platform) :
  command{0},
  channel{channel},
  platform{platform},
  path{path},
  root{root},
  terminating{false},
  recursive{recursive},
  buffer_size{DEFAULT_BUFFER_SIZE},
  buffer{new BYTE[buffer_size]},
  written{new BYTE[buffer_size]},
  old_path_seen{false}
{
  ZeroMemory(&overlapped, sizeof(OVERLAPPED));
  overlapped.hEvent = this;
}

Subscription::~Subscription()
{
  CloseHandle(root);
}

Result<bool> Subscription::schedule(LPOVERLAPPED_COMPLETION_ROUTINE fn)
{
  if (terminating) {
    return ok_result(true);
  }

  int success = ReadDirectoryChangesW(root,  // root directory handle
    buffer.get(),  // result buffer
    buffer_size,  // result buffer size
    recursive,  // recursive
    FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE
      | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION
      | FILE_NOTIFY_CHANGE_SECURITY,  // change flags
    NULL,  // bytes returned
    &overlapped,  // overlapped
    fn  // completion routine
  );

  if (!success) {
    DWORD last_error = GetLastError();
    if (last_error == ERROR_INVALID_FUNCTION) {
      // Filesystem does not support change events.
      return ok_result(false);
    }

    return windows_error_result<bool>("Unable to subscribe to filesystem events", last_error);
  }

  return ok_result(true);
}

Result<> Subscription::use_network_size()
{
  if (buffer_size <= NETWORK_BUFFER_SIZE) {
	stringbuilder out;
	out << ("Buffer size of ");
    out << buffer_size << " is already lower than the network buffer size " << NETWORK_BUFFER_SIZE;
    return error_result(out.mstr());
  }

  buffer_size = NETWORK_BUFFER_SIZE;
  buffer.reset(new BYTE[buffer_size]);
  written.reset(new BYTE[buffer_size]);

  return ok_result();
}

BYTE *Subscription::get_written(DWORD written_size)
{
  memcpy(written.get(), buffer.get(), written_size);
  return written.get();
}

Result<string> Subscription::get_root_path()
{
  return to_utf8(path);
}

wstring Subscription::make_absolute(const wstring &sub_path)
{
  wstring p = path;

  if (path.back() != L'\\' && path.back() != L'/' && sub_path.front() != L'\\') {
	  p.push_back( L'\\');
  }
  p.append(sub_path);

  return p;
}

Result<> Subscription::stop(const CommandID cmd)
{
  if (terminating) return ok_result();

  bool success = CancelIo(root);
  if (!success) return windows_error_result<>("Unable to cancel pending I/O");

  terminating = true;
  command = cmd;

  return ok_result();
}
