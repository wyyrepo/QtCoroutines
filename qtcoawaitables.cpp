#include "qtcoawaitables.h"
#include <QTimer>
#include <QDebug>

QtCoroutine::timeout::timeout(std::chrono::milliseconds tout) :
	_timeout{std::move(tout)}
{}

void QtCoroutine::timeout::prepare(std::function<void()> resume)
{
	QTimer::singleShot(_timeout, std::move(resume));
}

QtCoroutine::timeout::type QtCoroutine::timeout::result() {}



QtCoroutine::iodevice::iodevice(QIODevice *device, qint64 readCnt) :
	_device{device},
	_readCnt{readCnt}
{
	Q_ASSERT_X(_device->isOpen() && _device->isReadable(), Q_FUNC_INFO, "device must be open and readable");
	if(_device->openMode().testFlag(QIODevice::Unbuffered))
		qWarning() << "Unbuffered devices may not properly work and never return!";
}

void QtCoroutine::iodevice::prepare(std::function<void()> resume)
{
	_connection = QObject::connect(_device, &QIODevice::readyRead,
								   [this, resume{std::move(resume)}]() {
		switch(_readCnt) {
		case ReadAll:
			_result = _device->readAll();
			break;
		case ReadLine:
			if(!_device->canReadLine())
				return;
			_result = _device->readLine();
			break;
		default:
			if(_device->bytesAvailable() < _readCnt)
				return;
			_result = _device->read(_readCnt);
			break;
		}
		QObject::disconnect(_connection);
		resume();
	});
}

QtCoroutine::iodevice::type &&QtCoroutine::iodevice::result()
{
	return std::move(_result);
}
