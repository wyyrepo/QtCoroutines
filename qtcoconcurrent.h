#ifndef QTCOCONCURRENT_H
#define QTCOCONCURRENT_H

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include "qtcoroutine.h"

namespace QtCoroutine {

template <typename T>
struct future {
	future(const QFuture<T> &fut) {
		_watcher.setFuture(fut);
	}

	using type = T;
	void prepare(std::function<void()> resume) {
		QObject::connect(&_watcher, &QFutureWatcherBase::finished,
						 [resume{std::move(resume)}]() {
			resume();
		});
	}
	type result() {
		return _watcher.result();
	}

private:
	QFutureWatcher<T> _watcher;
};

template <typename T>
inline T await(const QFuture<T> &fut) {
	return await(future<T>{fut});
}

template <typename T>
struct fn_info : public fn_info<decltype(&T::operator())> {};

template <typename TClass, typename TRet, typename... TArgs>
struct fn_info<TRet(TClass::*)(TArgs...) const>
{
	using ReturnType = TRet;
};

template <typename TFunction>
typename fn_info<TFunction>::ReturnType asyncAwait(TFunction fn) {
	return await(future<typename fn_info<TFunction>::ReturnType>{QtConcurrent::run(std::move(fn))});
}

template <typename TFunction>
typename fn_info<TFunction>::ReturnType asyncAwait(QThreadPool *pool, TFunction fn) {
	return await(future<typename fn_info<TFunction>::ReturnType>{QtConcurrent::run(pool, std::move(fn))});
}

}

#endif // QTCOCONCURRENT_H
