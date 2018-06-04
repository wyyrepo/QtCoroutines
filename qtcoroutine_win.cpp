#include "qtcoroutine_p.h"
#include <QDebug>

QtCoroutine::ResumeResult QtCoroutine::resume(QtCoroutine::RoutineId id)
{
	if(!Ordinator::ordinator.routines.contains(id))
		return Finished;

	{
		// convert main thread to fiber
		if(!Ordinator::ordinator.fiber) {
			Ordinator::ordinator.fiber = ConvertThreadToFiber(nullptr);
			if(!Ordinator::ordinator.fiber) {
				qWarning() << "Failed to create fiber with errno:" << qt_error_string(GetLastError());
				return Error;
			}
		}

		// get the target fiber (scoped)
		LPVOID targetFiber = nullptr;
		{
			auto &routine = Ordinator::ordinator.routines[id];
			if(routine.fiber.isNull()) {
				routine.fiber.reset(CreateFiber(StackSize, &Ordinator::entry, nullptr),
									DeleteFiber);
				if(routine.fiber.isNull()) {
					qWarning() << "Failed to create fiber with errno:" << qt_error_string(GetLastError());
					return Error;
				}
			}
			targetFiber = routine.fiber.data();
			Ordinator::ordinator.executionStack.push({id, routine});
		}

		// perform the switch
		SwitchToFiber(targetFiber);
		Ordinator::ordinator.executionStack.pop();
	}

	return Ordinator::ordinator.routines.contains(id) ? Paused : Finished;
}

void QtCoroutine::yield()
{
	if(Ordinator::ordinator.executionStack.isEmpty())
		return; // not in a coroutine
	SwitchToFiber(Ordinator::ordinator.previous());
}

QtCoroutine::Ordinator::ContextType QtCoroutine::Ordinator::previous()
{
	if(executionStack.size() <= 1)
		return &fiber;
	else
		return executionStack[executionStack.size() - 2].second.fiber.data();
}

void QtCoroutine::Ordinator::entry()
{
	{
		auto routineInfo = ordinator.executionStack.top();
		routineInfo.second.func();
		ordinator.routines.remove(routineInfo.first);
	}
	SwitchToFiber(Ordinator::ordinator.previous());
}
