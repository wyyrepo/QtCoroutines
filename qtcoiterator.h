#ifndef QTCOITERATOR_H
#define QTCOITERATOR_H

#include <QStack>
#include "qtcoroutine.h"

namespace QtCoroutine {

template <typename T>
void yield_return(T data);

template <typename T>
class Iterator
{
public:
	Iterator(RoutineId producer);
	Iterator(std::function<void()> producer);
	Iterator(const Iterator<T> &other) = default;
	Iterator(Iterator<T> &&other) = default;

	Iterator<T> &operator=(const Iterator<T> &other) = default;
	Iterator<T> &operator=(Iterator<T> &&other) = default;

	bool hasNext();
	T next();

private:
	friend void QtCoroutine::yield_return<T>(T);

	RoutineId _producer;
	bool _routineExists;
	bool _hasCurrent = false;
	T _current;

	bool getNext();
};

// internal helper class
class IteratorContainer
{
private:
	template <typename T>
	friend class Iterator;
	template <typename T>
	friend void QtCoroutine::yield_return(T);

	template <typename T>
	static inline void push(Iterator<T> *iterator) {
		_consumers.push(iterator);
	}
	template <typename T>
	static inline Iterator<T> *top() {
		Q_ASSERT_X(!_consumers.isEmpty(), Q_FUNC_INFO, "yield_return must only be used in coroutines called from an QtCoroutine::Iterator");
		return reinterpret_cast<Iterator<T>*>(_consumers.top());
	}
	static inline void pop() {
		_consumers.pop();
	}

	thread_local static QStack<void*> _consumers;
};



template<typename T>
Iterator<T>::Iterator(RoutineId producer) :
	_producer{producer},
	_routineExists{QtCoroutine::isPaused(_producer)}
{
	Q_ASSERT_X(_producer != InvalidRoutineId, Q_FUNC_INFO, "The producer parameter must be a valid producer");
}

template<typename T>
Iterator<T>::Iterator(std::function<void()> producer) :
	Iterator<T>{QtCoroutine::create(std::move(producer))}
{}

template<typename T>
bool Iterator<T>::hasNext()
{
	return getNext();
}

template<typename T>
T Iterator<T>::next()
{
	if(!getNext())
		;//TODO throw
	_hasCurrent = false;
	return _current;
}

template<typename T>
bool Iterator<T>::getNext()
{
	if(_hasCurrent)
		return true;
	if(!_routineExists)
		return false;
	IteratorContainer::push(this);
	_routineExists = (QtCoroutine::resume(_producer) == QtCoroutine::Paused);
	IteratorContainer::pop();
	return _hasCurrent;
}

template <typename T>
void yield_return(T data)
{
	auto consumer = IteratorContainer::top<T>();
	Q_ASSERT_X(!consumer->_hasCurrent, Q_FUNC_INFO, "Do not resume an iterator method manually");
	consumer->_current = std::move(data);
	consumer->_hasCurrent = true;
	QtCoroutine::yield();
}

}

#endif // QTCOITERATOR_H
