#ifndef QTCOROUTINE_H
#define QTCOROUTINE_H

#include <functional>
#include <chrono>

#include <QAtomicInt>
#include <QObject>

namespace QtCoroutine
{

// standard coroutine methods, as wrappers around the header lib

using RoutineId = quint64;
static const RoutineId InvalidRoutineId = 0;

enum ResumeResult {
	Finished,
	Paused,
	Error
};

extern QAtomicInteger<size_t> StackSize;

RoutineId create(std::function<void()> fn);
bool isPaused(RoutineId id);
ResumeResult resume(RoutineId id);
void cancel(RoutineId id);
std::pair<RoutineId, ResumeResult> createAndRun(std::function<void()> fn);

RoutineId current();
void yield();
void abort();

template <typename TAwaitable>
typename TAwaitable::type await(TAwaitable &&awaitable) {
	awaitable.prepare();
	yield();
	return awaitable.result();
};

};

#endif // QTCOROUTINE_H
