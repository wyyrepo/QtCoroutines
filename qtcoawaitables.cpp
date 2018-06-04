#include "qtcoawaitables.h"
#include <QTimer>

QtCoroutine::timeout::timeout(std::chrono::milliseconds tout) :
	_timeout{std::move(tout)}
{}

void QtCoroutine::timeout::prepare()
{
	const auto routine = current();
	QTimer::singleShot(_timeout, std::bind(&QtCoroutine::resume, routine));
}

QtCoroutine::timeout::type QtCoroutine::timeout::result() {}

void QtCoroutine::await(std::chrono::milliseconds tout)
{
	await(timeout{std::move(tout)});
}
