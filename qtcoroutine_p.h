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
	QSharedPointer<VOID> fiber{nullptr, [](LPVOID x){DeleteFiber(x);}};
#else
	QSharedPointer<char> stack;
	QSharedPointer<ucontext_t> ctx{new ucontext_t{}};
#endif

	Routine(std::function<void()> &&func = {});
};

struct Ordinator
{
	QHash<RoutineId, Routine> routines;
	QStack<QPair<RoutineId, Routine>> executionStack;
#ifdef Q_OS_WIN
	LPVOID fiber;
	using ContextType = LPVOID;
#else
	ucontext_t ctx;
	using ContextType = ucontext_t*;
#endif

	ContextType previous();

	thread_local static Ordinator ordinator;
	static QAtomicInteger<RoutineId> index;

	static void entry();
};

}

#endif // QTCOROUTINE_P_H
