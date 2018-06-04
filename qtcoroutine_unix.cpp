#include "qtcoroutine_p.h"
#include <QDebug>

QtCoroutine::ResumeResult QtCoroutine::resume(QtCoroutine::RoutineId id)
{
	if(!Ordinator::ordinator.routines.contains(id))
		return Finished;

	{
		auto currentCtx = Ordinator::ordinator.executionStack.isEmpty() ?
							  &Ordinator::ordinator.ctx :
							  Ordinator::ordinator.executionStack.top().second.ctx.data();
		ucontext_t *targetCtx = nullptr;

		// get the current context (scoped)
		{
			auto &routine = Ordinator::ordinator.routines[id];
			if(routine.stack.isNull()) {
				if(getcontext(routine.ctx.data()) != 0) {
					qWarning() << "Failed to get context with errno:" << errno;
					return Error;
				}

				//Before invoking makecontext(), the caller must allocate a new stack
				//for this context and assign its address to ucp->uc_stack,
				//and define a successor context and assign its address to ucp->uc_link.
				routine.stack.reset(new char[StackSize], [](char*x){delete[]x;});
				routine.ctx->uc_stack.ss_sp = routine.stack.data();
				routine.ctx->uc_stack.ss_size = StackSize;
				routine.ctx->uc_link = &Ordinator::ordinator.ctx; //"emergency switch back"

				//When this context is later activated by swapcontext(), the function entry is called.
				//When this function returns, the  successor context is activated.
				//If the successor context pointer is NULL, the thread exits.
				makecontext(routine.ctx.data(), &Ordinator::entry, 0);
			}
			targetCtx = routine.ctx.data();
			Ordinator::ordinator.executionStack.push({id, routine});
		}

		//The swapcontext() function saves the current context,
		//and then activates the context of another.
		auto res = swapcontext(currentCtx, targetCtx);
		Ordinator::ordinator.executionStack.pop();
		if(res != 0) {
			qWarning() << "Failed to swap into new context with errno:" << errno;
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
		qWarning() << "Failed to swap back to previous context with errno:" << errno;
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
