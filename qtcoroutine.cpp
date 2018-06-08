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


bool QtCoroutine::isPaused(QtCoroutine::RoutineId id)
{
	return id != current() && Ordinator::ordinator.routines.contains(id);
}

QtCoroutine::ResumeResult QtCoroutine::resume(QtCoroutine::RoutineId id)
{
	if(!Ordinator::ordinator.routines.contains(id))
		return Finished;
	if(!Ordinator::ordinator.resume(id))
		return Error;
	return Ordinator::ordinator.routines.contains(id) ? Paused : Finished;
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

void QtCoroutine::yield()
{
	if(Ordinator::ordinator.executionStack.isEmpty())
		return; // not in a coroutine
	Ordinator::ordinator.yield();
}

void QtCoroutine::abort()
{
	Q_ASSERT_X(current() != InvalidRoutineId, Q_FUNC_INFO, "QtCoroutine::abort can only be called from within a coroutine");
	Ordinator::ordinator.routines.remove(current());
	yield();
}



QtCoroutine::Routine::Routine(std::function<void()> &&func) :
	func{std::move(func)}
{}



thread_local QtCoroutine::Ordinator QtCoroutine::Ordinator::ordinator;
QAtomicInteger<quint64> QtCoroutine::Ordinator::index = 0;

QtCoroutine::Ordinator::ContextType QtCoroutine::Ordinator::previous()
{
	if(executionStack.size() <= 1)
		return context;
	else
		return executionStack[executionStack.size() - 2].second.context.data();
}

void QtCoroutine::Ordinator::entryImpl()
{
	auto id = executionStack.top().first;
	auto fn = executionStack.top().second.func;
	fn();
	routines.remove(id);
}
