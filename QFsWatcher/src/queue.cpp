#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "message.h"
#include "queue.h"
#include "result.h"

using std::move;
using std::string;
using std::unique_ptr;
using std::vector;

Queue::Queue() : active{new vector<Message>}
{
  freeze();
}

Queue::~Queue()
{
}

void Queue::enqueue(Message &&message)
{
    std::lock_guard<std::mutex> lock(mutex);
  active->push_back(move(message));
}

unique_ptr<vector<Message>> Queue::accept_all()
{
    std::lock_guard<std::mutex> lock(mutex);

  if (active->empty()) {
    unique_ptr<vector<Message>> n;
    return n;
  }

  unique_ptr<vector<Message>> consumed = move(active);
  active.reset(new vector<Message>);
  return consumed;
}

size_t Queue::size()
{
    std::lock_guard<std::mutex> lock(mutex);
  return active->size();
}
