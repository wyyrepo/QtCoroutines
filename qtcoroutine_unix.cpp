#include "qtcoroutine_p.h"
#include <QDebug>

bool QtCoroutine::Ordinator::resume(QtCoroutine::RoutineId id)
{
	// get the current context
	auto currentCtx = executionStack.isEmpty() ?
						  &context :
						  executionStack.top().second.context.data();

	// get the target context (scoped)
	ucontext_t *targetCtx = nullptr;
	{
		auto &routine = routines[id];
		if(routine.stack.isNull()) {
			if(getcontext(routine.context.data()) != 0) {
				qWarning() << "Failed to get context with errno:" << qt_error_string(errno);
				return false;
			}

#ifdef QTCO_DIAGNOSTICS
			static int cnt = 0;
			++cnt;
			routine.stack.reset(new char[StackSize], [&, id](char*x){delete[]x;qDebug() << "[CTXCNT]" << id << "cleared, remaining:" << --cnt;});
#else
			routine.stack.reset(new char[StackSize], [](char*x){delete[]x;});
#endif
			routine.context->uc_stack.ss_sp = routine.stack.data();
			routine.context->uc_stack.ss_size = StackSize;
			routine.context->uc_link = &context; //"emergency switch back"

			makecontext(routine.context.data(), &Ordinator::entry, 0);
		}
		targetCtx = routine.context.data();
		executionStack.push({id, routine});
	}

	auto res = swapcontext(currentCtx, targetCtx);
	executionStack.pop();
	if(res != 0) {
		qWarning() << "Failed to swap into new context with errno:" << qt_error_string(errno);
		return false;
	}

	return true;
}

void QtCoroutine::Ordinator::yield()
{
	auto currentCtx = executionStack.top().second.context.data();
	auto prevCtx = previous();
	if(swapcontext(currentCtx, prevCtx) != 0)
		qWarning() << "Failed to swap back to previous context with errno:" << qt_error_string(errno);
}

void QtCoroutine::Ordinator::entry()
{
	ordinator.entryImpl();
	setcontext(ordinator.previous());
}
