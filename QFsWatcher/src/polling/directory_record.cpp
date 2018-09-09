#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <utility>

#include "../helper/common.h"
#include "../log.h"
#include "../message.h"
#include "../helper/qt.h"
#include "directory_record.h"
#include "polling_iterator.h"

using std::move;
using std::ostringstream;
using std::set;
using std::shared_ptr;
using std::string;

DirectoryRecord::DirectoryRecord(string &&prefix) :
  parent{nullptr},
  name{move(prefix)},
  populated{false},
  was_present{false}
{
  //
}

string DirectoryRecord::path() const
{
  return parent == nullptr ? name : path_join(parent->path(), name);
}

void DirectoryRecord::scan(BoundPollingIterator *it)
{
  set<Entry> scanned_entries;

  string dir = path();
  QDir qdir = QDir(QString::fromStdString(dir));
  if(!qdir.exists()){
      if (was_present) {
        entry_deleted(it, dir, KIND_DIRECTORY);
        was_present = false;
      }
      return;
  }

  if (!was_present) {
    entry_created(it, dir, KIND_DIRECTORY);
    was_present = true;
  }

  for(auto &&fileInfo : qdir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks))
  {
      EntryKind entry_kind = KIND_UNKNOWN;
      if(fileInfo.isFile()) entry_kind = KIND_FILE;
      if(fileInfo.isDir()) entry_kind = KIND_DIRECTORY;
      string entry_name = fileInfo.fileName().toStdString();
      it->push_entry(string(entry_name), entry_kind);
      if (populated) scanned_entries.emplace(move(entry_name), entry_kind);
  }

  {
    // Report entries that were present the last time we scanned this directory, but aren't included in this
    // scan.
    auto previous = entries.begin();
    while (previous != entries.end()) {
      const string &previous_entry_name = previous->first;
      const string previous_entry_path(path_join(dir, previous_entry_name));
      EntryKind previous_entry_kind = kind_from_stat(previous->second);
      Entry previous_entry(previous_entry_name, previous_entry_kind);
      Entry unknown_entry(previous_entry_name, KIND_UNKNOWN);

      if (scanned_entries.count(previous_entry) == 0 && scanned_entries.count(unknown_entry) == 0) {
        entry_deleted(it, previous_entry_path, previous_entry_kind);
        auto former = previous;
        ++previous;

        subdirectories.erase(previous_entry_name);
        entries.erase(former);
      } else {
        ++previous;
      }
    }
  }
}

void DirectoryRecord::entry(BoundPollingIterator *it,
  const string &entry_name,
  const string &entry_path,
  EntryKind scan_kind)
{
  QT_STATBUF stat;
  EntryKind previous_kind = scan_kind;
  EntryKind current_kind = scan_kind;

  int lstat_err = QT_LSTAT(entry_path.c_str(), &stat);
  if (lstat_err != 0 && lstat_err != ENOENT && lstat_err != EACCES) {
    ostringstream msg;
    msg << "Unable to stat " << entry_path << ": " << strerror(lstat_err);
    it->get_buffer().error(msg.str(), false);
  }

  auto previous = entries.find(entry_name);

  bool existed_before = previous != entries.end();
  bool exists_now = lstat_err == 0;

  if (existed_before) previous_kind = kind_from_stat(previous->second);
  if (exists_now) current_kind = kind_from_stat(stat);

  if (existed_before && exists_now) {
    // Modification or no change
    QT_STATBUF &previous_stat = previous->second;
    QT_STATBUF &current_stat = stat;

    // TODO consider modifications to mode or ownership bits?
    if (kinds_are_different(previous_kind, current_kind) || previous_stat.st_ino != current_stat.st_ino) {
      entry_deleted(it, entry_path, previous_kind);
      entry_created(it, entry_path, current_kind);
    } else if (previous_stat.st_mode != current_stat.st_mode || previous_stat.st_size != current_stat.st_size
      || previous_stat.st_mtime != current_stat.st_mtime
      || previous_stat.st_ctime != current_stat.st_ctime) {
      entry_modified(it, entry_path, current_kind);
    }

  } else if (existed_before && !exists_now) {
    // Deletion

    entry_deleted(it, entry_path, previous_kind);

  } else if (!existed_before && exists_now) {
    // Creation

    if (kinds_are_different(scan_kind, current_kind)) {
      // Entry was created as a file, deleted, then recreated as a directory between scan() and entry()
      // (or vice versa)
      entry_created(it, entry_path, scan_kind);
      entry_deleted(it, entry_path, scan_kind);
    }
    entry_created(it, entry_path, current_kind);

  } else if (!existed_before && !exists_now) {
    // Entry was deleted between scan() and entry().
    // Emit a deletion and creation event pair. Note that the kinds will likely both be KIND_UNKNOWN.

    entry_created(it, entry_path, previous_kind);
    entry_deleted(it, entry_path, current_kind);
  }

  // Update entries with the latest stat information
  if (existed_before) entries.erase(previous);
  if (exists_now) entries.emplace(entry_name, stat);

  // Update subdirectories if this is or was a subdirectory
  auto dir = subdirectories.find(entry_name);
  if (current_kind != KIND_DIRECTORY && current_kind != KIND_UNKNOWN && dir != subdirectories.end()) {
    subdirectories.erase(dir);
  }
  if (current_kind == KIND_DIRECTORY && it->is_recursive()) {
    if (dir == subdirectories.end()) {
      shared_ptr<DirectoryRecord> subdir(new DirectoryRecord(this, string(entry_name)));
      subdirectories.emplace(entry_name, subdir);
      it->push_directory(subdir);
    } else {
      it->push_directory(dir->second);
    }
  }
}

bool DirectoryRecord::all_populated() const
{
  if (!populated) return false;

  for (auto &pair : subdirectories) {
    if (!pair.second->all_populated()) {
      return false;
    }
  }

  return true;
}

size_t DirectoryRecord::count_entries() const
{
  // Start with 1 to count the readdir() on this directory.
  size_t count = 1;
  for (auto &pair : entries) {
    if ((pair.second.st_mode & S_IFDIR) != S_IFDIR) {
      count++;
    }
  }
  for (auto &pair : subdirectories) {
    count += pair.second->count_entries();
  }
  return count;
}

DirectoryRecord::DirectoryRecord(DirectoryRecord *parent, string &&name) :
  parent{parent},
  name(move(name)),
  populated{false},
  was_present{false}
{
  //
}

void DirectoryRecord::entry_deleted(BoundPollingIterator *it, const string &entry_path, EntryKind kind)
{
  if (!populated) return;

  it->get_buffer().deleted(string(entry_path), kind);
}

void DirectoryRecord::entry_created(BoundPollingIterator *it, const string &entry_path, EntryKind kind)
{
  if (!populated) return;

  it->get_buffer().created(string(entry_path), kind);
}

void DirectoryRecord::entry_modified(BoundPollingIterator *it, const string &entry_path, EntryKind kind)
{
  if (!populated) return;

  it->get_buffer().modified(string(entry_path), kind);
}
