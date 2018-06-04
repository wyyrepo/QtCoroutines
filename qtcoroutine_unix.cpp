#include "qtcoroutine_p.h"
#include <QDebug>

QtCoroutine::ResumeResult QtCoroutine::resume(QtCoroutine::RoutineId id)
{
	if(!Ordinator::ordinator.routines.contains(id))
		return Finished;

	{
		// get the current context
		auto currentCtx = Ordinator::ordinator.executionStack.isEmpty() ?
							  &Ordinator::ordinator.ctx :
							  Ordinator::ordinator.executionStack.top().second.ctx.data();

		// get the target context (scoped)
		ucontext_t *targetCtx = nullptr;
		{
			auto &routine = Ordinator::ordinator.routines[id];
			if(routine.stack.isNull()) {
				if(getcontext(routine.ctx.data()) != 0) {
					qWarning() << "Failed to get context with errno:" << qt_error_string(errno);
					return Error;
				}

				routine.stack.reset(new char[StackSize], [](char*x){delete[]x;});
				routine.ctx->uc_stack.ss_sp = routine.stack.data();
				routine.ctx->uc_stack.ss_size = StackSize;
				routine.ctx->uc_link = &Ordinator::ordinator.ctx; //"emergency switch back"

				makecontext(routine.ctx.data(), &Ordinator::entry, 0);
			}
			targetCtx = routine.ctx.data();
			Ordinator::ordinator.executionStack.push({id, routine});
		}

		auto res = swapcontext(currentCtx, targetCtx);
		Ordinator::ordinator.executionStack.pop();
		if(res != 0) {
			qWarning() << "Failed to swap into new context with errno:" << qt_error_string(errno);
			return Error;
		}
	}

	return Ordinator::ordinator.routines.contains(id) ? Paused : Finished;
}

void QtCoroutine::yield()
{
	if(Ordinator::ordinator.executionStack.isEmpty())
		return; // not in a coroutine

	auto currentCtx = Ordinator::ordinator.executionStack.top().second.ctx.data();
	auto prevCtx = Ordinator::ordinator.previous();
	if(swapcontext(currentCtx, prevCtx) != 0)
		qWarning() << "Failed to swap back to previous context with errno:" << qt_error_string(errno);
}

QtCoroutine::Ordinator::ContextType QtCoroutine::Ordinator::previous()
{
	if(executionStack.size() <= 1)
		return &ctx;
	else
		return executionStack[executionStack.size() - 2].second.ctx.data();
}

void QtCoroutine::Ordinator::entry()
{
	{
		auto routineInfo = ordinator.executionStack.top();
		routineInfo.second.func();
		ordinator.routines.remove(routineInfo.first);
	}
	setcontext(ordinator.previous());
}
