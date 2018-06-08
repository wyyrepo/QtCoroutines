#include "qtcoroutine_p.h"
#include <QDebug>

bool QtCoroutine::Ordinator::resume(QtCoroutine::RoutineId id)
{
	// convert main thread to fiber
	if(!context) {
		context = ConvertThreadToFiber(nullptr);
		if(!context) {
			qWarning() << "Failed to create fiber with errno:" << qt_error_string(GetLastError());
			return false;
		}
	}

	// get the target fiber (scoped)
	LPVOID targetFiber = nullptr;
	{
		auto &routine = routines[id];
		if(routine.context.isNull()) {
			routine.context.reset(reinterpret_cast<Routine::Fiber*>(CreateFiber(StackSize, &Ordinator::entry, nullptr)),
								  DeleteFiber);
			if(routine.context.isNull()) {
				qWarning() << "Failed to create fiber with errno:" << qt_error_string(GetLastError());
				return false;
			}
		}
		targetFiber = routine.context.data();
		executionStack.push({id, routine});
	}

	// perform the switch
	SwitchToFiber(targetFiber);
	executionStack.pop();
	return true;
}

void QtCoroutine::Ordinator::yield()
{
	SwitchToFiber(previous());
}

void WINAPI QtCoroutine::Ordinator::entry(LPVOID)
{
	ordinator.entryImpl();
	SwitchToFiber(ordinator.previous());
}
