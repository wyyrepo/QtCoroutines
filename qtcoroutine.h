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
	awaitable.prepare(std::bind(&QtCoroutine::resume, current()));
	yield();
	return awaitable.result();
};

// theoretical template interface for await
// You don't have to implement this interface - it is only ment as documentation.
// Any class you implement that provides members with the same signatures (generic or non generic doesn't matter) can be passed to await
// You can still use this interface though if you want to enforce someone to implement an awaitable
template <typename T>
class Awaitable
{
public:
	using type = T;
	virtual void prepare(std::function<void()>) = 0;
	virtual type result() = 0;
};

};

#endif // QTCOROUTINE_H
