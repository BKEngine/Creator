#include "QFuzzyMatcher.h"
#include "score_match.h"

#include <algorithm>
#include <queue>

using namespace std;

typedef priority_queue<MatchResult> ResultHeap;

inline int letter_bitmask(const QString &str) {
	int result = 0;
	for (int i = 0; i < str.size(); i++) {
		if (str[i] >= 'a' && str[i] <= 'z') {
			result |= (1 << (str[i].unicode() - 'a'));
		}
	}
	return result;
}

inline QString str_to_lower(const QString &s) {
	return s.toLower();
}

// Push a new entry on the heap while ensuring size <= max_results.
void push_heap(ResultHeap &heap,
	float score,
	const QString *value)
{
	heap.emplace(score, value);
}

QVector<MatchResult> finalize(const QString &query,
	const QString &query_case,
	const MatchOptions &options,
	bool record_match_indexes,
	ResultHeap &&heap) {
	QVector<MatchResult> vec;
	while (heap.size()) {
		const MatchResult &result = heap.top();
		if (record_match_indexes) {
			result.matchIndexes.reset(new QVector<int>(query.size()));
			QString lower = str_to_lower(*result.value);
			score_match(
				*result.value,
				lower,
				query,
				query_case,
				options,
				result.matchIndexes.data()
			);
		}
		vec.push_back(result);
		heap.pop();
	}
	reverse(vec.begin(), vec.end());
	return vec;
}

void worker(
	const QString &query,
	const QString &query_case,
	const MatchOptions &options,
	const QVector<QFuzzyMatcher::CandidateData> &candidates,
	ResultHeap &result
) {
	int bitmask = letter_bitmask(query_case);
	for (size_t i = 0; i < candidates.size(); i++) {
		const auto &candidate = candidates[i];
		if ((bitmask & candidate.bitmask) == bitmask) {
			float score = score_match(
				candidate.value,
				candidate.lowercase,
				query,
				query_case,
				options
			);
			if (score > 0) {
				push_heap(result, score, &candidate.value);
			}
		}
	}
}


QFuzzyMatcher::QFuzzyMatcher(const QStringList &candidates)
{
	for (const QString &s : candidates)
	{
		addCandidate(s);
	}
}

QVector<MatchResult> QFuzzyMatcher::findMatches(const QString &query,
	const MatcherOptions &options) {
	MatchOptions matchOptions;
	matchOptions.case_sensitive = options.case_sensitive;
	matchOptions.smart_case = false;
	matchOptions.max_gap = options.max_gap;

	QString new_query;
	// Ignore all whitespace in the query.
	for (auto c : query) {
		if (!c.isSpace()) {
			new_query += c;
		}
		if (c.isUpper() && !matchOptions.case_sensitive) {
			matchOptions.smart_case = true;
		}
	}

	QString query_case;
	if (!options.case_sensitive) {
		query_case = str_to_lower(new_query);
	}
	else {
		query_case = query;
	}

	ResultHeap combined;
	worker(new_query, query_case, matchOptions,
		candidates_, combined);
	return finalize(
		new_query,
		query_case,
		matchOptions,
		options.record_match_indexes,
		move(combined)
	);
}

void QFuzzyMatcher::addCandidate(const QString &candidate) {
	QString lowercase = str_to_lower(candidate);
	CandidateData data;
	data.value = candidate;
	data.bitmask = letter_bitmask(lowercase);
	data.lowercase = move(lowercase);
	candidates_.append(move(data));
}