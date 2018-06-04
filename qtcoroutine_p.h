#ifndef QTCOROUTINE_P_H
#define QTCOROUTINE_P_H

#include "qtcoroutine.h"
#include <QStack>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#else
#include <ucontext.h>
#include <QSharedPointer>
#endif

namespace QtCoroutine {

struct Routine
{
	std::function<void()> func;
	//TODO remove deleters from here, they are set when the vars are set
#ifdef Q_OS_WIN
	QSharedPointer<VOID> context;
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
	ucontext_t context;
	using ContextType = ucontext_t*;
#endif

	bool resume(RoutineId id);
	void yield();
	ContextType previous();
	void entryImpl();

	thread_local static Ordinator ordinator;
	static QAtomicInteger<RoutineId> index;

	static void entry();
};

}

#endif // QTCOROUTINE_P_H
