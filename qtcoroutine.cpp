#include "qtcoroutine.h"

#define STACK_LIMIT QtCoroutine::StackSize
#include "coroutine/coroutine.h"

QAtomicInt QtCoroutine::StackSize{1024*1024};

QtCoroutine::RoutineId QtCoroutine::create(std::function<void ()> fn)
{
	return coroutine::create(std::move(fn));
}

void QtCoroutine::destroy(QtCoroutine::RoutineId id)
{
	coroutine::destroy(id);
}

QtCoroutine::ResumeResult QtCoroutine::resume(QtCoroutine::RoutineId id)
{
	return static_cast<ResumeResult>(coroutine::resume(id));
}

QtCoroutine::RoutineId QtCoroutine::current()
{
	return coroutine::current();
}

void QtCoroutine::yield()
{
	coroutine::yield();
}



QtCoroutine::RoutineId::RoutineId() = default;

QtCoroutine::RoutineId::RoutineId(quint32 id) :
	id{id}
{}

QtCoroutine::RoutineId::operator quint32() const
{
	return id;
}

QtCoroutine::RoutineId::operator bool() const
{
	return id == 0;
}

bool QtCoroutine::RoutineId::operator!() const
{
	return id != 0;
}

bool QtCoroutine::RoutineId::isValid() const
{
	return id == 0;
}
