#pragma once

#include <QMap>

namespace details {
	struct _QSortedSetDummyStruct
	{
	};
}

template<class Key>
class QSortedSet : public QMap<Key, details::_QSortedSetDummyStruct>
{
	typedef details::_QSortedSetDummyStruct Dummy;
	typedef QMap<Key, details::_QSortedSetDummyStruct> Base;
	using Base::take;
	using Base::key;
	using Base::value;
	using Base::operator[];
	using Base::values;
	using Base::keyBegin;
	using Base::keyEnd;
	using Base::constBegin;
	using Base::constEnd;
	using Base::cbegin;
	using Base::cend;
	using Base::find;
	using Base::firstKey;
	using Base::lastKey;
	using Base::constFind;
	using Base::lowerBound;
	using Base::upperBound;
	using Base::insertMulti;
public:
        inline QSortedSet() Q_DECL_NOTHROW : Base() {}
#ifdef Q_COMPILER_INITIALIZER_LISTS
	inline QSortedSet(std::initializer_list<Key> list)
		: QSortedSet()
	{
		for (typename std::initializer_list<Key>::const_iterator it = list.begin(); it != list.end(); ++it)
			insert(*it);
	}
#endif

        QSortedSet(const QSortedSet &other) : Base(other) {}

	inline QSortedSet &operator=(const QSortedSet &other) {
		Base::operator =(other);
		return *this;
	}

#ifdef Q_COMPILER_RVALUE_REFS
	inline QSortedSet &operator=(QSortedSet &&other) Q_DECL_NOTHROW
	{
		Base::operator =(qMove(other));
		return *this;
	}
#endif

        typedef typename Base::key_iterator const_iterator;

	inline const_iterator begin() const { return Base::keyBegin(); }
	inline const_iterator end() const { return Base::keyEnd(); }
	inline const Key &first() const { return Base::firstKey(); }
	inline const Key &last() const { return Base::lastKey(); }
        inline const_iterator insert(const Key &key) { return const_iterator(typename Base::const_iterator(Base::insert(key, Dummy()))); }
	inline const_iterator insert(const_iterator pos, const Key &key) { return Base::insert(pos.base(), key, Dummy()); }
};
