#include "MatcherBase.h"
#include "score_match.h"

#include <algorithm>
#include <queue>
#include <thread>

using namespace std;

typedef priority_queue<MatchResult> ResultHeap;

inline int letter_bitmask(const char *str) {
  int result = 0;
  for (int i = 0; str[i]; i++) {
    if (str[i] >= 'a' && str[i] <= 'z') {
      result |= (1 << (str[i] - 'a'));
    }
  }
  return result;
}

inline string str_to_lower(const std::string &s) {
  string lower(s);
  for (auto& c : lower) {
    if (c >= 'A' && c <= 'Z') {
      c += 'a' - 'A';
    }
  }
  return lower;
}

// Push a new entry on the heap while ensuring size <= max_results.
void push_heap(ResultHeap &heap,
               float score,
               const std::string *value,
               size_t max_results) {
  if (heap.size() < max_results || score > heap.top().score) {
    heap.emplace(score, value);
    if (heap.size() > max_results) {
      heap.pop();
    }
  }
}

vector<MatchResult> finalize(const string &query,
                             const string &query_case,
                             const MatchOptions &options,
                             bool record_match_indexes,
                             ResultHeap &&heap) {
  vector<MatchResult> vec;
  while (heap.size()) {
    const MatchResult &result = heap.top();
    if (record_match_indexes) {
      result.matchIndexes.reset(new vector<int>(query.size()));
      string lower = str_to_lower(*result.value);
      score_match(
        result.value->c_str(),
        lower.c_str(),
        query.c_str(),
        query_case.c_str(),
        options,
        result.matchIndexes.get()
      );
    }
    vec.push_back(result);
    heap.pop();
  }
  reverse(vec.begin(), vec.end());
  return vec;
}

void thread_worker(
  const string &query,
  const string &query_case,
  const MatchOptions &options,
  size_t max_results,
  const vector<MatcherBase::CandidateData> &candidates,
  size_t start,
  size_t end,
  ResultHeap &result
) {
  int bitmask = letter_bitmask(query_case.c_str());
  for (size_t i = start; i < end; i++) {
    const auto &candidate = candidates[i];
    if ((bitmask & candidate.bitmask) == bitmask) {
      float score = score_match(
        candidate.value.c_str(),
        candidate.lowercase.c_str(),
        query.c_str(),
        query_case.c_str(),
        options
      );
      if (score > 0) {
        push_heap(result, score, &candidate.value, max_results);
      }
    }
  }
}

vector<MatchResult> MatcherBase::findMatches(const std::string &query,
                                             const MatcherOptions &options) {
  size_t max_results = options.max_results;
  size_t num_threads = options.num_threads;
  if (max_results == 0) {
    max_results = numeric_limits<size_t>::max();
  }
  MatchOptions matchOptions;
  matchOptions.case_sensitive = options.case_sensitive;
  matchOptions.smart_case = false;
  matchOptions.max_gap = options.max_gap;

  string new_query;
  // Ignore all whitespace in the query.
  for (auto c : query) {
    if (!isspace(c)) {
      new_query += c;
    }
    if (isupper(c) && !matchOptions.case_sensitive) {
      matchOptions.smart_case = true;
    }
  }

  string query_case;
  if (!options.case_sensitive) {
    query_case = str_to_lower(new_query);
  } else {
    query_case = query;
  }

  ResultHeap combined;
  if (num_threads == 0 || candidates_.size() < 10000) {
    thread_worker(new_query, query_case, matchOptions, max_results,
                  candidates_, 0, candidates_.size(), combined);
  } else {
    vector<ResultHeap> thread_results(num_threads);
    vector<thread> threads;
    size_t cur_start = 0;
    for (size_t i = 0; i < num_threads; i++) {
      size_t chunk_size = candidates_.size() / num_threads;
      // Distribute remainder among the chunks.
      if (i < candidates_.size() % num_threads) {
        chunk_size++;
      }
      threads.emplace_back(
        thread_worker,
        ref(new_query),
        ref(query_case),
        ref(matchOptions),
        max_results,
        ref(candidates_),
        cur_start,
        cur_start + chunk_size,
        ref(thread_results[i])
      );
      cur_start += chunk_size;
    }

    for (size_t i = 0; i < num_threads; i++) {
      threads[i].join();
      while (thread_results[i].size()) {
        auto &top = thread_results[i].top();
        push_heap(combined, top.score, top.value, max_results);
        thread_results[i].pop();
      }
    }
  }

  return finalize(
    new_query,
    query_case,
    matchOptions,
    options.record_match_indexes,
    move(combined)
  );
}

void MatcherBase::addCandidate(const string &candidate) {
  auto it = lookup_.find(candidate);
  if (it == lookup_.end()) {
    string lowercase = str_to_lower(candidate);
    lookup_[candidate] = candidates_.size();
    CandidateData data;
    data.value = candidate;
    data.bitmask = letter_bitmask(lowercase.c_str());
    data.lowercase = move(lowercase);
    candidates_.emplace_back(move(data));
  }
}

void MatcherBase::removeCandidate(const string &candidate) {
  auto it = lookup_.find(candidate);
  if (it != lookup_.end()) {
    if (it->second + 1 != candidates_.size()) {
      swap(candidates_[it->second], candidates_.back());
      lookup_[candidates_[it->second].value] = it->second;
    }
    candidates_.pop_back();
    lookup_.erase(candidate);
  }
}

void MatcherBase::clear() {
  candidates_.clear();
  lookup_.clear();
}

void MatcherBase::reserve(size_t n) {
  candidates_.reserve(n);
  lookup_.reserve(n);
}

size_t MatcherBase::size() const {
  return candidates_.size();
}
