#ifndef QTAWAITABLES_H
#define QTAWAITABLES_H

#include "qtcoroutine.h"
#include <QIODevice>

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

inline void await(std::chrono::milliseconds tout) {
	await(timeout{std::move(tout)});
}
template <typename _Rep, typename _Period>
inline void await(const std::chrono::duration<_Rep, _Period> &tout) {
	await(timeout{std::chrono::duration_cast<std::chrono::milliseconds>(tout)});
}

//signals

template <typename Func, typename... Args>
struct signal {
	signal(const typename QtPrivate::FunctionPointer<Func>::Object *sender, Func signal) :
		_sender{std::move(sender)},
		_signal{std::move(signal)}
	{}

	using type = std::tuple<std::decay_t<Args>...>;
	void prepare() {
		const auto routine = current();
		_connection = QObject::connect(_sender, _signal,
									   [this, routine](std::decay_t<Args>... args) {
			_result = std::make_tuple(args...);
			QObject::disconnect(_connection);
			resume(routine);
		});
	}
	type &&result() {
		return std::move(_result);
	}

private:
	const typename QtPrivate::FunctionPointer<Func>::Object *_sender;
	const Func _signal;
	QMetaObject::Connection _connection;
	type _result;
};

template<typename, typename>
struct sigargs {};

template<typename TFunc, typename... Args>
struct sigargs<TFunc, QtPrivate::List<Args...>> : signal<TFunc, Args...> {
	inline sigargs(const typename QtPrivate::FunctionPointer<TFunc>::Object *sender, TFunc &&signal) :
		QtCoroutine::signal<TFunc, Args...>{std::move(sender), std::move(signal)}
	{}
};

template<typename TFunc, typename = QtPrivate::FunctionPointer<TFunc>>
struct sigfn {};

template<typename TFunc>
struct sigfn<TFunc, QtPrivate::FunctionPointer<TFunc>> : sigargs<TFunc, typename QtPrivate::FunctionPointer<TFunc>::Arguments> {
	inline sigfn(const typename QtPrivate::FunctionPointer<TFunc>::Object *sender, TFunc &&signal) :
		QtCoroutine::sigargs<TFunc, typename QtPrivate::FunctionPointer<TFunc>::Arguments>{std::move(sender), std::move(signal)}
	{}
};

template<typename... Args>
struct awaitargs {
	template <typename Func>
	static inline std::tuple<Args...> tawait(const typename QtPrivate::FunctionPointer<Func>::Object *sender, Func signal) {
		return QtCoroutine::await(QtCoroutine::signal<Func, Args...>{std::move(sender), std::move(signal)});
	}

	template <typename Func>
	static inline std::tuple<Args...> await(const typename QtPrivate::FunctionPointer<Func>::Object *sender, Func signal) {
		return QtCoroutine::await(QtCoroutine::signal<Func, Args...>{std::move(sender), std::move(signal)});
	}
};

template<typename Arg>
struct awaitargs<Arg> {
	template <typename Func>
	static inline std::tuple<Arg> tawait(const typename QtPrivate::FunctionPointer<Func>::Object *sender, Func signal) {
		return QtCoroutine::await(QtCoroutine::signal<Func, Arg>{std::move(sender), std::move(signal)});
	}

	template <typename Func>
	static inline Arg await(const typename QtPrivate::FunctionPointer<Func>::Object *sender, Func signal) {
		return std::get<0>(QtCoroutine::await(QtCoroutine::signal<Func, Arg>{std::move(sender), std::move(signal)}));
	}
};

template <typename Func, typename = std::enable_if_t<QtPrivate::FunctionPointer<Func>::ArgumentCount != -1>>
inline typename sigfn<Func>::type await(const typename QtPrivate::FunctionPointer<Func>::Object *sender, Func signal)
{
	return await(QtCoroutine::sigfn<Func>{std::move(sender), std::move(signal)});
}

// io device

struct iodevice {
	enum SpecialReads : qint64 {
		ReadAll = -1,
		ReadLine = 0
	};

	iodevice(QIODevice *device, qint64 readCnt = ReadAll);

	using type = QByteArray;
	void prepare();
	type &&result();

private:
	QIODevice * const _device;
	const qint64 _readCnt;
	QMetaObject::Connection _connection;
	QByteArray _result;
};

inline QByteArray await(QIODevice *device, qint64 readCnt) {
	return await(iodevice{device, readCnt});
}
inline QByteArray awaitAll(QIODevice *device) {
	return await(iodevice{device, iodevice::ReadAll});
}
inline QByteArray awaitLine(QIODevice *device) {
	return await(iodevice{device, iodevice::ReadLine});
}

}

#endif // QTAWAITABLES_H
