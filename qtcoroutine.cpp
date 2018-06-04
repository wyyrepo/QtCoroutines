#include "qtcoroutine.h"
#include "qtcoroutine_p.h"

QAtomicInteger<size_t> QtCoroutine::StackSize{1024*1024};

QtCoroutine::RoutineId QtCoroutine::create(std::function<void ()> fn)
{
	quint64 id;
	do {
		id = ++Ordinator::index;
	} while(id == 0 && Ordinator::ordinator.routines.contains(id));
	Ordinator::ordinator.routines.insert(id, std::move(fn));
	return id;
}

void QtCoroutine::cancel(QtCoroutine::RoutineId id)
{
	Q_ASSERT(id != current());
	Ordinator::ordinator.routines.remove(id);
	for(auto it = Ordinator::ordinator.executionStack.begin(), end = Ordinator::ordinator.executionStack.end(); it != end; it++) {
		if(it->first == id) {
			Ordinator::ordinator.executionStack.erase(it);
			break;
		}
	}
}

std::pair<QtCoroutine::RoutineId, QtCoroutine::ResumeResult> QtCoroutine::createAndRun(std::function<void ()> fn)
{
	auto id = create(std::move(fn));
	return std::make_pair(id, resume(id));
}

QtCoroutine::RoutineId QtCoroutine::current()
{
	return Ordinator::ordinator.executionStack.isEmpty() ?
				InvalidRoutineId :
				Ordinator::ordinator.executionStack.top().first;
}



thread_local QtCoroutine::Ordinator QtCoroutine::Ordinator::ordinator;
QAtomicInteger<quint64> QtCoroutine::Ordinator::index = 0;

QtCoroutine::Routine::Routine(std::function<void()> &&func) :
	func{std::move(func)}
{}

void QtCoroutine::abort()
{
	Ordinator::ordinator.routines.remove(current());
	yield();
}
