#pragma once

#include <QChar>
#include <QVector>

struct MatchOptions {
  bool case_sensitive;
  bool smart_case;
  size_t max_gap;
};

/**
 * Returns a matching score between 0-1.
 * 0 represents no match at all, while 1 is a perfect match.
 * See implementation for scoring details.
 *
 * If options.case_sensitive is false, haystack_lower and
 * needle_lower must be provided.
 *
 * If match_indexes is non-null, the optimal match index in haystack
 * will be computed for each value in needle (when score is non-zero).
 */
float score_match(const QString &haystack,
                  const QString &haystack_lower,
                  const QString &needle,
                  const QString &needle_lower,
                  const MatchOptions &options,
                  QVector<int> *match_indexes = nullptr);
