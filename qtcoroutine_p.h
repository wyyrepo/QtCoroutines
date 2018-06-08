#ifndef QTCOROUTINE_P_H
#define QTCOROUTINE_P_H

#include "qtcoroutine.h"
#include <QStack>
#include <QSharedPointer>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#else
#include <ucontext.h>
#endif

namespace QtCoroutine {

struct Routine
{
	std::function<void()> func;
#ifdef Q_OS_WIN
	struct Fiber {};
	QSharedPointer<Fiber> context;
#else
	QSharedPointer<char> stack;
	QSharedPointer<ucontext_t> context{new ucontext_t{}};
#endif

	Routine(std::function<void()> &&func = {});
};

struct Ordinator
{
	QHash<RoutineId, Routine> routines;
	QStack<QPair<RoutineId, Routine>> executionStack;
#ifdef Q_OS_WIN
	LPVOID context;
	using ContextType = LPVOID;
#else
	ucontext_t mContext;
	ucontext_t* const context = &mContext;
	using ContextType = ucontext_t*;
#endif

	bool resume(RoutineId id);
	void yield();
	ContextType previous();
	void entryImpl();

	thread_local static Ordinator ordinator;
	static QAtomicInteger<RoutineId> index;

#ifdef Q_OS_WIN
	static void WINAPI entry(LPVOID);
#else
	static void entry();
#endif
};

}

#endif // QTCOROUTINE_P_H
