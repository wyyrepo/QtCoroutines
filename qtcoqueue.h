#ifndef QTCOQUEUE_H
#define QTCOQUEUE_H

#include <QQueue>
#include "qtcoroutine.h"

namespace QtCoroutine {

template <typename T>
class Queue
{
	Q_DISABLE_COPY(Queue)
public:
	Queue() = default;

	void enqueue(const T &data);
	T &head();
	const T &head() const;
	T dequeue();

private:
	QQueue<T> _queue;
	RoutineId _consumer = QtCoroutine::InvalidRoutineId;

	void fetch();
};



template<typename T>
void Queue<T>::enqueue(const T &data)
{
	_queue.enqueue(data);
	if(_consumer != QtCoroutine::InvalidRoutineId)
		QtCoroutine::resume(_consumer);
}

template<typename T>
T &Queue<T>::head()
{
	fetch();
	return _queue.head();
}

template<typename T>
const T &Queue<T>::head() const
{
	fetch();
	return _queue.head();
}

template<typename T>
T Queue<T>::dequeue()
{
	fetch();
	return _queue.dequeue();
}

template<typename T>
void Queue<T>::fetch()
{
	Q_ASSERT_X(_consumer == QtCoroutine::InvalidRoutineId, Q_FUNC_INFO, "there can always be only one consumer of a QtCoroutine::Queue");
	_consumer = QtCoroutine::current();
	Q_ASSERT_X(_consumer != QtCoroutine::InvalidRoutineId, Q_FUNC_INFO, "QtCoroutine::Queue can only be read from a coroutine");
	while(_queue.isEmpty())
		QtCoroutine::yield();
	_consumer = QtCoroutine::InvalidRoutineId;
}

}

#endif // QTCOQUEUE_H
