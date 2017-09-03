#pragma once
#include <QString>
#include <QHash>
#include <QSharedPointer>
#include <QVector>

struct MatcherOptions {
	bool case_sensitive = false;
	size_t max_gap = 0;
	bool record_match_indexes = false;
};

struct MatchResult {
	float score;
	// We can't afford to copy strings around while we're ranking them.
	// These are not guaranteed to last very long and should be copied out ASAP.
	const QString *value;
	// Only computed if `record_match_indexes` was set to true.
	mutable QSharedPointer<QVector<int>> matchIndexes;

	MatchResult(float score = 0.f, const QString *value = nullptr)
		: score(score), value(value) {}

	// Order small scores to the top of any priority queue.
	// We need a min-heap to maintain the top-N results.
	bool operator<(const MatchResult& other) const {
		// In case of a tie, favour shorter strings.
		if (score == other.score) {
			return value->length() < other.value->length();
		}
		return score > other.score;
	}
};

class QFuzzyMatcher {
public:
	struct CandidateData {
		QString value;
		QString lowercase;
		/**
		* A bitmask of the letters (a-z) contained in the string.
		* ('a' = 1, 'b' = 2, 'c' = 4, ...)
		* We can then compute the bitmask of the query and very quickly prune out
		* non-matches in many practical cases.
		*/
		int bitmask;
	};

	QFuzzyMatcher(const QStringList &candidates);

	QVector<MatchResult> findMatches(const QString &query,
		const MatcherOptions &options);

private:
	// Storing candidate data in an array makes table scans significantly faster.
	// This makes add/remove slightly more expensive, but in our case queries
	// are significantly more frequent.
	QVector<CandidateData> candidates_;
	void addCandidate(const QString &candidate);
};
