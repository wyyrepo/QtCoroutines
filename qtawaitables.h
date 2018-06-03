#ifndef QTAWAITABLES_H
#define QTAWAITABLES_H

#include "qtcoroutine.h"

namespace QtCoroutine {

// timeout

struct timeout {
	timeout(std::chrono::milliseconds tout);
	template <typename _Rep, typename _Period>
	timeout(const std::chrono::duration<_Rep, _Period> &tout) :
		timeout{std::chrono::duration_cast<std::chrono::milliseconds>(tout)}
	{}

	using type = void;
	void prepare();
	type result();

private:
	const std::chrono::milliseconds _timeout;
};

void await(std::chrono::milliseconds tout);
template <typename _Rep, typename _Period>
void await(const std::chrono::duration<_Rep, _Period> &tout) {
	await(timeout{std::chrono::duration_cast<std::chrono::milliseconds>(tout)});
}

//signals

template <typename Func, typename... Args>
struct signal {
	signal(const typename QtPrivate::FunctionPointer<Func>::Object *sender, Func signal) :
		_sender{std::move(sender)},
		_signal{std::move(signal)}
	{}

	using type = std::tuple<Args...>;
	void prepare() {
		const auto routine = current();
		const auto connection = QObject::connect(_sender, _signal,
												 [this, routine](Args... args) {
			return signalTriggered(routine, args...);
		});
	}
	type result() {
		return _result;
	}

private:
	const typename QtPrivate::FunctionPointer<Func>::Object *_sender;
	const Func _signal;
	QMetaObject::Connection _connection;
	type _result;

	void signalTriggered(RoutineId id, Args... args) {
		_result = std::make_tuple(args...);
		QObject::disconnect(_connection);
		_connection = {};
		resume(id);
	}
};

template<typename, typename>
struct sigargs {};

template<typename TFunc, typename... Args>
struct sigargs<TFunc, QtPrivate::List<Args...>> : signal<TFunc, Args...> {
	sigargs(const typename QtPrivate::FunctionPointer<TFunc>::Object *sender, TFunc &&signal) :
		QtCoroutine::signal<TFunc, Args...>{std::move(sender), std::move(signal)}
	{}
};

template<typename TFunc, typename = QtPrivate::FunctionPointer<TFunc>>
struct sigfn {};

template<typename TFunc>
struct sigfn<TFunc, QtPrivate::FunctionPointer<TFunc>> : sigargs<TFunc, typename QtPrivate::FunctionPointer<TFunc>::Arguments> {
	sigfn(const typename QtPrivate::FunctionPointer<TFunc>::Object *sender, TFunc &&signal) :
		QtCoroutine::sigargs<TFunc, typename QtPrivate::FunctionPointer<TFunc>::Arguments>{std::move(sender), std::move(signal)}
	{}
};

template<typename... Args>
struct awaitargs {
	template <typename Func>
	static std::tuple<Args...> tawait(const typename QtPrivate::FunctionPointer<Func>::Object *sender, Func signal) {
		return QtCoroutine::await(QtCoroutine::signal<Func, Args...>{std::move(sender), std::move(signal)});
	}

	template <typename Func>
	static std::tuple<Args...> await(const typename QtPrivate::FunctionPointer<Func>::Object *sender, Func signal) {
		return QtCoroutine::await(QtCoroutine::signal<Func, Args...>{std::move(sender), std::move(signal)});
	}
};

template<typename Arg>
struct awaitargs<Arg> {
	template <typename Func>
	static std::tuple<Arg> tawait(const typename QtPrivate::FunctionPointer<Func>::Object *sender, Func signal) {
		return QtCoroutine::await(QtCoroutine::signal<Func, Arg>{std::move(sender), std::move(signal)});
	}

	template <typename Func>
	static Arg await(const typename QtPrivate::FunctionPointer<Func>::Object *sender, Func signal) {
		return std::get<0>(QtCoroutine::await(QtCoroutine::signal<Func, Arg>{std::move(sender), std::move(signal)}));
	}
};

template <typename Func>
typename sigfn<Func>::type await(const typename QtPrivate::FunctionPointer<Func>::Object *sender, Func signal)
{
	return await(QtCoroutine::sigfn<Func>{std::move(sender), std::move(signal)});
}

}

#endif // QTAWAITABLES_H
