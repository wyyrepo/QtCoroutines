#include <QCoreApplication>
#include <QDebug>
#include <QThread>

#include "qtcoroutine.h"
#include "qtawaitables.h"

using namespace QtCoroutine;
using namespace std::chrono_literals;

namespace {

QObject *testObj;

}

void coroutine_1()
{
	qDebug() << "begin" << Q_FUNC_INFO;
	yield();
	qDebug() << "end" << Q_FUNC_INFO;
}

void coroutine_2()
{
	qDebug() << "begin" << Q_FUNC_INFO;

	auto tpl = awaitargs<QString>::tawait(testObj, &QObject::objectNameChanged);
	static_assert(std::is_same<decltype(tpl), std::tuple<QString>>::value, "tpl wrong");
	qDebug() << "name changed to tuple:" << std::get<0>(tpl);

	auto str = awaitargs<QString>::await(testObj, &QObject::objectNameChanged);
	static_assert(std::is_same<decltype(str), QString>::value, "str wrong");
	qDebug() << "name changed to string:" << str;

	auto obj = await(testObj, &QObject::destroyed);
	static_assert(std::is_same<decltype(obj), std::tuple<QObject*>>::value, "obj wrong");
	qDebug() << "object was destroyed:" << std::get<0>(obj);

	awaitargs<>::await(qApp, &QCoreApplication::aboutToQuit);
	qDebug() << "end" << Q_FUNC_INFO;
}

void coroutine_3()
{
	qDebug() << "begin" << Q_FUNC_INFO;
	await(3s);
	QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
	qDebug() << "end" << Q_FUNC_INFO;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	// test simple yield
	auto id = createAndRun(coroutine_1).first;
	qDebug() << "back in main";
	resume(id);
	qDebug() << "back in main";

	// test await signals
	testObj = new QObject(qApp);
	createAndRun(coroutine_2);
	qDebug() << "changing name to test1";
	testObj->setObjectName("test1");
	qDebug() << "changing name to test2";
	testObj->setObjectName("test2");
	qDebug() << "destroying object";
	delete testObj;

	// test await timeout
	createAndRun(coroutine_3);
	return a.exec();
}
