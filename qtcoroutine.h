#ifndef QTCOROUTINE_H
#define QTCOROUTINE_H

#include <functional>
#include <chrono>

#include <QAtomicInt>
#include <QObject>

namespace QtCoroutine
{

// standard coroutine methods, as wrappers around the header lib

struct RoutineId
{
	quint32 id = 0;

	RoutineId();
	RoutineId(quint32 id);

	operator quint32() const;
	operator bool() const;
	bool operator!() const;

	bool isValid() const;
};

enum ResumeResult {
	Finished = -2,
	InvalidId = -1,
	Paused = 0
};

extern QAtomicInt StackSize;

RoutineId create(std::function<void()> fn);
void destroy(RoutineId id);
ResumeResult resume(RoutineId id);

RoutineId current();
void yield();

template <typename TAwaitable>
typename TAwaitable::type await(TAwaitable &&awaitable) {
	awaitable.prepare();
	yield();
	return awaitable.result();
};

};

#endif // QTCOROUTINE_H
